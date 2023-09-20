{nixpkgs, dysnomia, disnix}:

let
  manifestTests = ./manifest;
  machine = import ./machine.nix { inherit dysnomia disnix; };
in
with import "${nixpkgs}/nixos/lib/testing-python.nix" { system = builtins.currentSystem; };

simpleTest {
  name = "dbus";

  nodes = {
    client = machine;
    server = machine;
  };
  testScript =
    let
      env = "NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no'";
    in
    ''
      start_all()

      manifest = client.succeed(
          "${env} disnix-manifest -s ${manifestTests}/services-composition.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-composition.nix"
      )
      closure = client.succeed("nix-store -qR {}".format(manifest)).split("\n")
      target1Profile = [c for c in closure if "-testtarget1" in c][0]

      #### Test disnix-client / disnix-service

      # Check invalid path. We query an invalid path from the service
      # which should return the path we have given.
      # This test should succeed.

      result = client.succeed(
          "disnix-client --print-invalid /nix/store/00000000000000000000000000000000-invalid"
      )

      if "/nix/store/00000000000000000000000000000000-invalid" in result:
          print("/nix/store/00000000000000000000000000000000-invalid is invalid")
      else:
          raise Exception(
              "/nix/store/00000000000000000000000000000000-invalid should be invalid"
          )

      # Check invalid path. We query a valid path from the service
      # which should return nothing in this case.
      # This test should succeed.

      result = client.succeed(
          "disnix-client --print-invalid ${pkgs.bash}"
      )

      if "bash" in result:
          raise Exception(
              "${pkgs.bash} should not be returned!"
          )
      else:
          print("${pkgs.bash} is valid")

      # Query requisites test. Queries the requisites of the bash shell
      # and checks whether it is part of the closure.
      # This test should succeed.

      result = client.succeed(
          "disnix-client --query-requisites ${pkgs.bash}"
      )

      if "bash" in result:
          print("${pkgs.bash} is in the closure")
      else:
          raise Exception(
              "${pkgs.bash} should be in the closure!"
          )

      # Realise test. First the coreutils derivation file is instantiated,
      # then it is realised. This test should succeed.

      result = client.succeed(
          "nix-instantiate ${nixpkgs} -A coreutils"
      )
      client.succeed("disnix-client --realise {}".format(result))

      # Set test. Adds the testtarget1 profile as only derivation into
      # the Disnix profile. We first set the profile, then we check
      # whether the profile is part of the closure.
      # This test should succeed.

      client.succeed("disnix-client --set --profile default {}".format(target1Profile))
      defaultProfileClosure = client.succeed(
          "nix-store -qR /nix/var/nix/profiles/disnix/default"
      ).split("\n")

      if target1Profile in defaultProfileClosure:
          print("{} is part of the closure".format(target1Profile))
      else:
          raise Exception("{} should be part of the closure".format(target1Profile))

      # Query installed test. Queries the installed services in the
      # profile, which has been set in the previous testcase.
      # testService1 should be in there. This test should succeed.

      closure = client.succeed("disnix-client --query-installed --profile default").split(
          "\n"
      )

      if any("testService1" in c for c in closure):
          print("testService1 is installed in the default profile")
      else:
          raise Exception("testService1 should be installed in the default profile")

      # Collect garbage test. This test should succeed.
      # Testcase disabled, as this is very expensive.
      # client.succeed("disnix-client --collect-garbage")

      # Export test. Exports the closure of the bash shell.
      # This test should succeed.
      result = client.succeed(
          "disnix-client --export ${pkgs.bash}"
      )

      # Import test. Imports the exported closure of the previous test.
      # This test should succeed.
      client.succeed("disnix-client --import {}".format(result))

      # Lock test. This test should succeed.
      client.succeed("disnix-client --lock")

      # Lock test. This test should fail, since the service instance is already locked
      client.fail("disnix-client --lock")

      # Unlock test. This test should succeed, so that we can release the lock
      client.succeed("disnix-client --unlock")

      # Unlock test. This test should fail as the lock has already been released
      client.fail("disnix-client --unlock")

      # Use the echo type to activate a service.
      # We use the testService1 service defined in the manifest earlier
      # This test should succeed.

      closure = client.succeed("nix-store -qR {}".format(manifest[:-1])).split("\n")
      testService1 = [c for c in closure if "-testService1" in c][0]

      client.succeed(
          "disnix-client --activate --arguments foo=foo --arguments bar=bar --type echo {}".format(
              testService1
          )
      )

      # Deactivate the same service using the echo type. This test should succeed.
      client.succeed(
          "disnix-client --deactivate --arguments foo=foo --arguments bar=bar --type echo {}".format(
              testService1
          )
      )

      # Security test. First we try to invoke a Disnix operation by an
      # unprivileged user, which should fail. Then we try the same
      # command by a privileged user, which should succeed.

      client.fail(
          "su - unprivileged -c 'disnix-client --print-invalid /nix/store/00000000000000000000000000000000-invalid'"
      )
      client.succeed(
          "su - privileged -c 'disnix-client --print-invalid /nix/store/00000000000000000000000000000000-invalid'"
      )

      # Logfiles test. We perform an operation and check the id of the
      # logfile. Then we stop the Disnix service and start it again and perform
      # another operation. It should create a logfile which id is one higher.

      client.succeed(
          "disnix-client --print-invalid /nix/store/00000000000000000000000000000000-invalid"
      )
      result = client.succeed("ls /var/log/disnix | sort -n | tail -1")
      client.stop_job("disnix")
      client.start_job("disnix")
      client.wait_for_unit("disnix")
      client.succeed(
          "sleep 3; disnix-client --print-invalid /nix/store/00000000000000000000000000000000-invalid"
      )
      result2 = client.succeed("ls /var/log/disnix | sort -n | tail -1")

      if int(result2[:-1]) - int(result[:-1]) == 1:
          print("The log file numbers are correct!")
      else:
          raise Exception("The logfile numbers are incorrect!")

      # Capture config test. We capture a config and the tempfile should
      # contain one property: "foo" = "bar";
      result = client.succeed("disnix-client --capture-config")
      client.succeed('(cat {}) | grep \'"foo" = "bar"\${"'"}'.format(result))
    '';
}
