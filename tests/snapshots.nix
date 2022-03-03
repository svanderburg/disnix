{nixpkgs, stdenv, dysnomia, disnix, disnixRemoteClient}:

let
  manifestTests = ./manifest;
  machine = import ./machine.nix { inherit dysnomia disnix; };
  wrapper = import ./snapshots/wrapper.nix { inherit stdenv dysnomia; } {};
in
with import "${nixpkgs}/nixos/lib/testing-python.nix" { system = builtins.currentSystem; };

simpleTest {
  nodes = {
    client = machine;
    server = machine;
  };
  testScript =
    let
      env = "SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' DISNIX_REMOTE_CLIENT=${disnixRemoteClient}";
    in
    ''
      import subprocess

      start_all()

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

      #### Test disnix-ssh-client's snapshot operations

      server.wait_for_unit("sshd")
      client.succeed("sleep 10")  # !!! Delay hack

      # Activate the wrapper component
      client.succeed(
          "${env} disnix-ssh-client --target server --activate --type wrapper ${wrapper}"
      )

      # Take a snapshot
      client.succeed(
          "${env} disnix-ssh-client --target server --snapshot --type wrapper ${wrapper}"
      )

      # Make a change and take another snapshot
      server.succeed("echo 1 > /var/db/wrapper/state")
      client.succeed(
          "${env} disnix-ssh-client --target server --snapshot --type wrapper ${wrapper}"
      )

      # Make another change and take yet another snapshot
      server.succeed("echo 2 > /var/db/wrapper/state")
      client.succeed(
          "${env} disnix-ssh-client --target server --snapshot --type wrapper ${wrapper}"
      )

      # Query all snapshots. We expect three of them.
      result = client.succeed(
          "${env} disnix-ssh-client --target server --query-all-snapshots --container wrapper --component ${wrapper} | wc -l"
      )

      if int(result) == 3:
          print("We have 3 snapshots!")
      else:
          raise Exception("Expecting 3 snapshots, but we have: {}".format(result))

      # Query latest snapshot. The resulting snapshot text must contain 2.
      result = client.succeed(
          "${env} disnix-ssh-client --target server --query-latest-snapshot --container wrapper --component ${wrapper}"
      )
      lastSnapshot = result[:-1]
      lastResolvedSnapshot = client.succeed(
          "${env} disnix-ssh-client --target server --resolve-snapshots {}".format(
              lastSnapshot
          )
      )
      result = server.succeed("cat {}/state".format(lastResolvedSnapshot[:-1]))

      if result == "2\n":
          print("Result is 2")
      else:
          raise Exception("Result should be 2!")

      # Print missing snapshots. The former path should exist while the latter
      # should not, so it should return only one snapshot.

      result = client.succeed(
          "${env} disnix-ssh-client --target server --print-missing-snapshots --container wrapper --component ${wrapper} $lastSnapshot wrapper/wrapper/foo | wc -l"
      )

      if int(result) == 1:
          print("Result is 1")
      else:
          raise Exception("Result should be 1!")

      # Export snapshot. This operation should fetch the latest snapshot from
      # the server, containing the string 2.

      result = client.succeed(
          "${env} disnix-ssh-client --target server --export-snapshots {}".format(
              lastResolvedSnapshot[:-1]
          )
      )
      tmpDir = client.succeed(
          "echo {result}/$(basename {lastSnapshot})".format(
              result=result[:-1], lastSnapshot=lastSnapshot
          )
      )
      result = client.succeed("cat {}/state".format(tmpDir[:-1]))

      if result == "2\n":
          print("Result is 2")
      else:
          raise Exception("Result should be 2!")

      # Make another change and take yet another snapshot
      server.succeed("echo 3 > /var/db/wrapper/state")
      client.succeed(
          "${env} disnix-ssh-client --target server --snapshot --type wrapper ${wrapper}"
      )

      # Run the garbage collector. After running it only one snapshot should
      # be left containing the string 3
      client.succeed(
          "${env} disnix-ssh-client --target server --clean-snapshots --container wrapper --component ${wrapper}"
      )
      result = client.succeed(
          "${env} disnix-ssh-client --target server --query-all-snapshots --container wrapper --component ${wrapper} | wc -l"
      )

      if int(result) == 1:
          print("We only 1 snapshot left!")
      else:
          raise Exception("Expecting 1 remaining snapshot!")

      result = client.succeed(
          "${env} disnix-ssh-client --target server --query-latest-snapshot --container wrapper --component ${wrapper}"
      )
      lastSnapshot = result[:-1]
      lastResolvedSnapshot = client.succeed(
          "${env} disnix-ssh-client --target server --resolve-snapshots {}".format(
              lastSnapshot
          )
      )
      result = server.succeed("cat {}/state".format(lastResolvedSnapshot[:-1]))

      if result == "3\n":
          print("Result is 3")
      else:
          raise Exception("Result should be 3!")

      # Import the snapshot that has been previously exported and check if
      # there are actually two snapshots present.
      client.succeed(
          "${env} disnix-ssh-client --target server --import-snapshots --localfile --container wrapper --component wrapper {}".format(
              tmpDir[:-1]
          )
      )

      result = client.succeed(
          "${env} disnix-ssh-client --target server --query-all-snapshots --container wrapper --component ${wrapper} | wc -l"
      )

      if int(result) == 2:
          print("Result is 2")
      else:
          raise Exception("Result should be 2!")

      # Make another change
      server.succeed("echo 4 > /var/db/wrapper/state")

      # Restore the last snapshot and check whether it has the previously
      # uploaded state (2)
      client.succeed(
          "${env} disnix-ssh-client --target server --restore --type wrapper ${wrapper}"
      )
      result = server.succeed("cat /var/db/wrapper/state")

      if result == "2\n":
          print("Result is 2")
      else:
          raise Exception("Result should be 2!")

      # Deactivate the component
      client.succeed(
          "${env} disnix-ssh-client --target server --deactivate --type wrapper ${wrapper}"
      )

      # Delete the state and check if it is not present anymore.
      client.succeed(
          "${env} disnix-ssh-client --target server --delete-state --type wrapper ${wrapper}"
      )
      server.succeed("[ ! -e /var/db/wrapper ]")

      # Delete all the snapshots on the server machine and check if none is
      # present.

      client.succeed(
          "${env} disnix-ssh-client --target server --clean-snapshots --container wrapper --component ${wrapper} --keep 0"
      )
      result = client.succeed(
          "${env} disnix-ssh-client --target server --query-all-snapshots --container wrapper --component ${wrapper} | wc -l"
      )

      if int(result) == 0:
          print("We have no remaining snapshots left!")
      else:
          raise Exception("Expecting no remaining snapshots!")

      #### Test disnix-copy-snapshots

      # Activate the wrapper component
      client.succeed(
          "${env} disnix-ssh-client --target server --activate --type wrapper ${wrapper}"
      )

      # Take a snapshot
      client.succeed(
          "${env} disnix-ssh-client --target server --snapshot --type wrapper ${wrapper}"
      )

      # Make a change and take another snapshot
      server.succeed("echo 1 > /var/db/wrapper/state")
      client.succeed(
          "${env} disnix-ssh-client --target server --snapshot --type wrapper ${wrapper}"
      )

      # Make another change and take yet another snapshot
      server.succeed("echo 2 > /var/db/wrapper/state")
      client.succeed(
          "${env} disnix-ssh-client --target server --snapshot --type wrapper ${wrapper}"
      )

      # Copy the latest snapshot from the server, check whether only one has
      # been sent and if it is the latest one (containing the string: 2)

      client.succeed(
          "${env} disnix-copy-snapshots --from --target server --container wrapper --component ${wrapper}"
      )
      result = client.succeed(
          "dysnomia-snapshots --query-all --container wrapper --component ${wrapper} | wc -l"
      )

      if int(result) == 1:
          print("We have only one snapshot!")
      else:
          raise Exception("Expecting only one snapshot!")

      lastSnapshot = client.succeed(
          "dysnomia-snapshots --query-latest --container wrapper --component ${wrapper}"
      )
      lastResolvedSnapshot = client.succeed(
          "dysnomia-snapshots --resolve {}".format(lastSnapshot[:-1])
      )
      result = client.succeed("cat {}/state".format(lastResolvedSnapshot[:-1]))

      if result == "2\n":
          print("Result is 2")
      else:
          raise Exception("Result should be 2!")

      # Copy all (remaining) snapshots from the server and check whether we
      # have 4 of them.
      client.succeed(
          "${env} disnix-copy-snapshots --from --target server --container wrapper --component ${wrapper} --all"
      )

      result = client.succeed(
          "dysnomia-snapshots --query-all --container wrapper --component ${wrapper} | wc -l"
      )

      if int(result) == 4:
          print("We have 4 snapshots!")
      else:
          raise Exception("Expecting 4 snapshots, but we have: {}!".format(result))

      # Delete all snapshots from the server
      client.succeed(
          "${env} disnix-ssh-client --target server --clean-snapshots --container wrapper --component ${wrapper} --keep 0"
      )

      # Copy the latest snapshot to the server, check whether only one has
      # been sent and if it is the latest one (containing the string: 2)

      client.succeed(
          "${env} disnix-copy-snapshots --to --target server --container wrapper --component ${wrapper}"
      )
      result = server.succeed(
          "dysnomia-snapshots --query-all --container wrapper --component ${wrapper} | wc -l"
      )

      if int(result) == 1:
          print("We have only one snapshot!")
      else:
          raise Exception("Expecting only one snapshot!")

      lastSnapshot = server.succeed(
          "dysnomia-snapshots --query-latest --container wrapper --component ${wrapper}"
      )
      lastResolvedSnapshot = server.succeed(
          "dysnomia-snapshots --resolve {}".format(lastSnapshot[:-1])
      )
      result = server.succeed("cat {}/state".format(lastResolvedSnapshot[:-1]))

      if result == "2\n":
          print("Result is 2")
      else:
          raise Exception("Result should be 2, instead it is: {}!".format(result))

      # Copy all (remaining) snapshots to the server and check whether we
      # have 4 of them. The last snapshot should still contain 2 (which
      # corresponds to the last local snapshot).
      client.succeed(
          "${env} disnix-copy-snapshots --to --target server --container wrapper --component ${wrapper} --all"
      )

      result = server.succeed(
          "dysnomia-snapshots --query-all --container wrapper --component ${wrapper} | wc -l"
      )

      if int(result) == 4:
          print("We have 4 snapshots!")
      else:
          raise Exception("Expecting only 4 snapshots, but we have: {}!".format(result))

      lastSnapshot = server.succeed(
          "dysnomia-snapshots --query-latest --container wrapper --component ${wrapper}"
      )
      lastResolvedSnapshot = server.succeed(
          "dysnomia-snapshots --resolve {}".format(lastSnapshot[:-1])
      )
      result = server.succeed("cat {}/state".format(lastResolvedSnapshot[:-1]))

      if result == "2\n":
          print("Result is 2")
      else:
          raise Exception("Result should be 2, instead it is: {}!".format(result))
    '';
}
