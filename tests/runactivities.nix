{nixpkgs, dysnomia, disnix}:

let
  manifestTests = ./manifest;
  machine = import ./machine.nix { inherit dysnomia disnix; enableMultiUser = false; };
in
with import "${nixpkgs}/nixos/lib/testing-python.nix" { system = builtins.currentSystem; };

simpleTest {
  name = "runactivities";

  nodes = {
    server = machine;
  };
  testScript =
    let
      env = "NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no'";
    in
    ''
      start_all()

      manifest = server.succeed(
          "${env} disnix-manifest -s ${manifestTests}/services-composition.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-composition.nix"
      )

      closure = server.succeed("nix-store -qR {}".format(manifest)).split("\n")
      target1Profile = [c for c in closure if "-testtarget1" in c][0]

      #### Test disnix-run-activity

      # Check invalid path. We query an invalid path from the service
      # which should return the path we have given.
      # This test should succeed.

      result = server.succeed(
          "disnix-run-activity --print-invalid /nix/store/00000000000000000000000000000000-invalid"
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

      result = server.succeed(
          "disnix-run-activity --print-invalid ${pkgs.bash}"
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

      result = server.succeed(
          "disnix-run-activity --query-requisites ${pkgs.bash}"
      )

      if "bash" in result:
          print("${pkgs.bash} is in the closure")
      else:
          raise Exception(
              "${pkgs.bash} should be in the closure!"
          )

      # Realise test. First the coreutils derivation file is instantiated,
      # then it is realised. This test should succeed.

      result = server.succeed(
          "nix-instantiate ${nixpkgs} -A coreutils"
      )
      server.succeed("disnix-run-activity --realise {}".format(result))

      # Set test. Adds the testtarget1 profile as only derivation into
      # the Disnix profile. We first set the profile, then we check
      # whether the profile is part of the closure.
      # This test should succeed.

      server.succeed("disnix-run-activity --set --profile default {}".format(target1Profile))
      defaultProfileClosure = server.succeed(
          "nix-store -qR /nix/var/nix/profiles/disnix/default"
      ).split("\n")

      if target1Profile in defaultProfileClosure:
          print("{} is part of the closure".format(target1Profile))
      else:
          raise Exception("{} should be part of the closure".format(target1Profile))

      # Query installed test. Queries the installed services in the
      # profile, which has been set in the previous testcase.
      # testService1 should be in there. This test should succeed.

      closure = server.succeed(
          "disnix-run-activity --query-installed --profile default"
      ).split("\n")

      if any("testService1" in c for c in closure):
          print("testService1 is installed in the default profile")
      else:
          raise Exception("testService1 should be installed in the default profile")

      # Collect garbage test. This test should succeed.
      # Testcase disabled, as this is very expensive.
      # server.succeed("disnix-run-activity --collect-garbage")

      # Export test. Exports the closure of the bash shell.
      # This test should succeed.
      result = server.succeed(
          "disnix-run-activity --export ${pkgs.bash}"
      )

      # Import test. Imports the exported closure of the previous test.
      # This test should succeed.
      server.succeed("disnix-run-activity --import {}".format(result))

      # Lock test. This test should succeed.
      server.succeed("disnix-run-activity --lock")

      # Lock test. This test should fail, since the service instance is already locked
      server.fail("disnix-run-activity --lock")

      # Unlock test. This test should succeed, so that we can release the lock
      server.succeed("disnix-run-activity --unlock")

      # Unlock test. This test should fail as the lock has already been released
      server.fail("disnix-run-activity --unlock")

      # Use the echo type to activate a service.
      # We use the testService1 service defined in the manifest earlier
      # This test should succeed.

      closure = server.succeed("nix-store -qR {}".format(manifest[:-1])).split("\n")
      testService1 = [c for c in closure if "-testService1" in c][0]

      server.succeed(
          "disnix-run-activity --activate --arguments foo=foo --arguments bar=bar --type echo {}".format(
              testService1
          )
      )

      # Deactivate the same service using the echo type. This test should succeed.
      server.succeed(
          "disnix-run-activity --deactivate --arguments foo=foo --arguments bar=bar --type echo {}".format(
              testService1
          )
      )

      # Capture config test. We capture a config and the tempfile should
      # contain one property: "foo" = "bar";
      result = server.succeed("disnix-run-activity --capture-config")
      server.succeed('(cat {}) | grep \'"foo" = "bar"\${"'"}'.format(result))
    '';
}
