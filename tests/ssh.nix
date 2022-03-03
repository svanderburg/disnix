{nixpkgs, dysnomia, disnix, disnixRemoteClient}:

let
  manifestTests = ./manifest;
  machine = import ./machine.nix { inherit dysnomia disnix; };
in
with import "${nixpkgs}/nixos/lib/testing-python.nix" { system = builtins.currentSystem; };

simpleTest {
  nodes = {
    client = machine;
    server = machine;
  };
  testScript =
    let
      env = "NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' DISNIX_REMOTE_CLIENT=${disnixRemoteClient}";
    in
    ''
      import subprocess

      start_all()

      manifest = client.succeed(
          "${env} disnix-manifest -s ${manifestTests}/services-composition.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-composition.nix"
      )
      closure = client.succeed("nix-store -qR {}".format(manifest)).split("\n")

      # Initialise ssh stuff by creating a key pair for communication
      key = subprocess.check_output(
          '${pkgs.openssh}/bin/ssh-keygen -t ecdsa -f key -N ""',
          shell=True,
      )

      server.succeed("mkdir -m 700 /root/.ssh")
      server.copy_from_host("key.pub", "/root/.ssh/authorized_keys")

      client.succeed("mkdir -m 700 /root/.ssh")
      client.copy_from_host("key", "/root/.ssh/id_dsa")
      client.succeed("chmod 600 /root/.ssh/id_dsa")

      #### Test disnix-ssh-client

      # Check invalid path. We query an invalid path from the service
      # which should return the path we have given.
      # This test should succeed.

      server.wait_for_unit("sshd")

      result = client.succeed(
          "${env} disnix-ssh-client --target server --print-invalid /nix/store/00000000000000000000000000000000-invalid"
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
          "${env} disnix-ssh-client --target server --print-invalid ${pkgs.bash}"
      )

      # Query requisites test. Queries the requisites of the bash shell
      # and checks whether it is part of the closure.
      # This test should succeed.

      result = client.succeed(
          "${env} disnix-ssh-client --target server --query-requisites ${pkgs.bash}"
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
      client.succeed(
          "${env} disnix-ssh-client --target server --realise {}".format(
              result
          )
      )

      # Export test. Exports the closure of the bash shell on the server
      # and then imports it on the client. This test should succeed.

      result = client.succeed(
          "${env} disnix-ssh-client --target server --export --remotefile ${pkgs.bash}"
      )
      client.succeed("nix-store --import < {}".format(result))

      # Repeat the same export operation, but now as a localfile. It should
      # export the same closure to a file. This test should succeed.
      result = client.succeed(
          "${env} disnix-ssh-client --target server --export --localfile ${pkgs.bash}"
      )
      server.succeed("[ -e {} ]".format(result))

      # Import test. Creates a closure of the target2Profile on the
      # client. Then it imports the closure into the Nix store of the
      # server. This test should succeed.

      target2Profile = [c for c in closure if "-testtarget2" in c][0]
      server.fail("nix-store --check-validity {}".format(target2Profile))
      client.succeed(
          "nix-store --export $(nix-store -qR {}) > /root/target2Profile.closure".format(
              target2Profile
          )
      )
      client.succeed(
          "${env} disnix-ssh-client --target server --import --localfile /root/target2Profile.closure"
      )
      server.succeed("nix-store --check-validity {}".format(target2Profile))

      # Do a remotefile import. It should import the bash closure stored
      # remotely. This test should succeed.
      server.succeed("nix-store --export $(nix-store -qR /bin/sh) > /root/bash.closure")
      client.succeed(
          "${env} disnix-ssh-client --target server --import --remotefile /root/bash.closure"
      )

      # Set test. Adds the testtarget2 profile as only derivation into
      # the Disnix profile. We first set the profile, then we check
      # whether the profile is part of the closure.
      # This test should succeed.

      client.succeed(
          "${env} disnix-ssh-client --target server --set --profile default {}".format(
              target2Profile
          )
      )
      defaultProfileClosure = server.succeed(
          "nix-store -qR /nix/var/nix/profiles/disnix/default"
      ).split("\n")

      if target2Profile in defaultProfileClosure:
          print("{} is part of the closure".format(target2Profile))
      else:
          raise Exception("{} should be part of the closure".format(target2Profile))

      # Query installed test. Queries the installed services in the
      # profile, which has been set in the previous testcase.
      # testService2 should be in there. This test should succeed.

      closure = client.succeed(
          "${env} disnix-ssh-client --target server --query-installed --profile default"
      ).split("\n")

      if any("testService2" in c for c in closure):
          print("testService2 is installed in the default profile")
      else:
          raise Exception("testService2 should be installed in the default profile")

      # Collect garbage test. This test should succeed.
      # Testcase disabled, as this is very expensive.
      # client.succeed("${env} disnix-ssh-client --target server --collect-garbage")

      # Lock test. This test should succeed.
      client.succeed(
          "${env} disnix-ssh-client --target server --lock"
      )

      # Lock test. This test should fail, since the service instance is already locked
      client.fail(
          "${env} disnix-ssh-client --target server --lock"
      )

      # Unlock test. This test should succeed, so that we can release the lock
      client.succeed(
          "${env} disnix-ssh-client --target server --unlock"
      )

      # Unlock test. This test should fail as the lock has already been released
      client.fail(
          "${env} disnix-ssh-client --target server --unlock"
      )

      closure = client.succeed("nix-store -qR {}".format(manifest)).split("\n")
      testService1 = [c for c in closure if "-testService1" in c][0]

      # Use the echo type to activate a service.
      # We use the testService1 service defined in the manifest earlier
      # This test should succeed.
      client.succeed(
          "${env} disnix-ssh-client --target server --activate --arguments foo=foo --arguments bar=bar --type echo {}".format(
              testService1
          )
      )

      # Deactivate the same service using the echo type. This test should succeed.
      client.succeed(
          "${env} disnix-ssh-client --target server --deactivate --arguments foo=foo --arguments bar=bar --type echo {}".format(
              testService1
          )
      )

      # Deactivate the same service using the echo type. This test should succeed.
      client.succeed(
          "${env} disnix-ssh-client --target server --deactivate --arguments foo=foo --arguments bar=bar --type echo {}".format(
              testService1
          )
      )

      # Capture config test. We capture a config and the tempfile should
      # contain one property: "foo" = "bar";
      client.succeed(
          "${env} disnix-ssh-client --target server --capture-config | grep '\"foo\" = \"bar\"'"
      )

      # Shell test. We run a shell session in which we create a tempfile,
      # then we check whether the file exists and contains 'foo'
      client.succeed(
          "${env} disnix-ssh-client --target server --shell --arguments foo=foo --arguments bar=bar --type echo --command 'echo $foo > /tmp/tmpfile' {}".format(
              testService1
          )
      )
      server.succeed("grep 'foo' /tmp/tmpfile")
    '';
}
