{nixpkgs, dysnomia, disnix}:

let
  snapshotTests = ./snapshots;
  machine = import ./machine.nix { inherit dysnomia disnix; };
in
with import "${nixpkgs}/nixos/lib/testing-python.nix" { system = builtins.currentSystem; };

simpleTest {
  nodes = {
    coordinator = machine;
    testtarget1 = machine;
    testtarget2 = machine;
  };
  testScript =
    let
      env = ''DYSNOMIA_STATEDIR=/root/dysnomia SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' NIX_PATH='nixpkgs=${nixpkgs}' dysnomia=\"$(dirname $(readlink -f $(type -p dysnomia)))/..\"'';
    in
    ''
      import subprocess

      start_all()

      # Initialise ssh stuff by creating a key pair for communication
      key = subprocess.check_output(
          '${pkgs.openssh}/bin/ssh-keygen -t ecdsa -f key -N ""',
          shell=True,
      )

      testtarget1.succeed("mkdir -m 700 /root/.ssh")
      testtarget1.copy_from_host("key.pub", "/root/.ssh/authorized_keys")

      testtarget2.succeed("mkdir -m 700 /root/.ssh")
      testtarget2.copy_from_host("key.pub", "/root/.ssh/authorized_keys")

      coordinator.succeed("mkdir -m 700 /root/.ssh")
      coordinator.copy_from_host("key", "/root/.ssh/id_dsa")
      coordinator.succeed("chmod 600 /root/.ssh/id_dsa")

      # Use disnix-env to perform a new installation.
      # This test should succeed.
      coordinator.succeed(
          "${env} disnix-env -s ${snapshotTests}/services-state.nix -i ${snapshotTests}/infrastructure.nix -d ${snapshotTests}/distribution-simple.nix"
      )

      # Check if both machines have state deployed
      result = testtarget1.succeed("cat /var/db/testService1/state")

      if result[:-1] == "0":
          print("result is: 0")
      else:
          raise Exception("result should be: 0, instead it is: {}".format(result[:-1]))

      result = testtarget2.succeed("cat /var/db/testService2/state")

      if result[:-1] == "0":
          print("result is: 0")
      else:
          raise Exception("result should be: 0, instead it is: {}".format(result[:-1]))

      # Delete the state of the services. Because none of them have been marked
      # as garbage, they should be kept.

      coordinator.succeed(
          "${env} disnix-delete-state /nix/var/nix/profiles/per-user/root/disnix-coordinator/default-1-link"
      )

      testtarget1.succeed("cat /var/db/testService1/state")
      testtarget2.succeed("cat /var/db/testService2/state")

      # Modify the state of each deployed service

      testtarget1.succeed("echo 1 > /var/db/testService1/state")
      testtarget2.succeed("echo 2 > /var/db/testService2/state")

      # Use disnix-env to reverse the location of the deployed services.
      coordinator.succeed(
          "${env} disnix-env -s ${snapshotTests}/services-state.nix -i ${snapshotTests}/infrastructure.nix -d ${snapshotTests}/distribution-reverse.nix"
      )

      # Check if the state is reversed
      result = testtarget1.succeed("cat /var/db/testService2/state")

      if result[:-1] == "2":
          print("result is: 2")
      else:
          raise Exception("result should be: 2, but it is: {}".format(result[:-1]))

      result = testtarget2.succeed("cat /var/db/testService1/state")

      if result[:-1] == "1":
          print("result is: 1")
      else:
          raise Exception("result should be: 1, but it is: {}".format(result[:-1]))

      # Take a snapshot of everything and check if we have
      # the one snapshot of each service
      coordinator.succeed("rm -R /root/dysnomia")
      coordinator.succeed(
          "${env} disnix-snapshot"
      )

      result = coordinator.succeed(
          "${env} dysnomia-snapshots --query-all --container wrapper --component testService1 | wc -l"
      )

      if int(result) == 1:
          print("result is: 1")
      else:
          raise Exception("result should be: 1, but it is: {}".format(result))

      result = coordinator.succeed(
          "${env} dysnomia-snapshots --query-all --container wrapper --component testService2 | wc -l"
      )

      if int(result) == 1:
          print("result is: 1")
      else:
          raise Exception("result should be: 1, but it is: {}".format(result))

      # Check if old state is present, delete it and check if it is actually
      # removed.

      testtarget1.succeed("cat /var/db/testService1/state")
      testtarget2.succeed("cat /var/db/testService2/state")

      coordinator.succeed(
          "${env} disnix-delete-state /nix/var/nix/profiles/per-user/root/disnix-coordinator/default-1-link"
      )

      testtarget1.fail("cat /var/db/testService1/state")
      testtarget2.fail("cat /var/db/testService2/state")

      # Delete all the snapshots on the target machines

      coordinator.succeed(
          "${env} disnix-clean-snapshots ${snapshotTests}/infrastructure.nix --keep 0"
      )

      result = testtarget1.succeed(
          "dysnomia-snapshots --query-all --container wrapper --component testService2 | wc -l"
      )

      if int(result) == 0:
          print("result is: 0")
      else:
          raise Exception("result should be: 0, but it is: {}".format(result))

      result = testtarget2.succeed(
          "dysnomia-snapshots --query-all --container wrapper --component testService1 | wc -l"
      )

      if int(result) == 0:
          print("result is: 0")
      else:
          raise Exception("result should be: 0, but it is: {}".format(result))

      # Modify the state, restore the snapshots and check whether they have
      # their old state again.

      testtarget1.succeed("echo 5 > /var/db/testService2/state")
      testtarget2.succeed("echo 6 > /var/db/testService1/state")

      coordinator.succeed(
          "${env} disnix-restore --no-upgrade"
      )

      result = testtarget1.succeed("cat /var/db/testService2/state")

      if result[:-1] == "2":
          print("result is: 2")
      else:
          raise Exception("result should be: 2, but it is: {}".format(result[:-1]))

      result = testtarget2.succeed("cat /var/db/testService1/state")

      if result[:-1] == "1":
          print("result is: 1")
      else:
          raise Exception("result should be: 1, but it is: {}".format(result[:-1]))

      # Clean all snapshots on the target and coordinator machines for the component filtering tests
      coordinator.succeed(
          "${env} disnix-clean-snapshots ${snapshotTests}/infrastructure.nix --keep 0"
      )
      coordinator.succeed(
          "${env} dysnomia-snapshots --gc --keep 0"
      )

      # Only take a snapshot of testService1 and check whether this is the case
      coordinator.succeed(
          "${env} disnix-snapshot --component testService1"
      )
      snapshots = coordinator.succeed(
          "${env} dysnomia-snapshots --query-all --container wrapper"
      ).split("\n")

      if len(snapshots[:-1]) == 1:
          print("snapshots length is: 1")
      else:
          raise Exception(
              "snapshot length should be: 1, but it is: {}!".format(len(snapshots[:-1]))
          )

      if any("testService1" in s for s in snapshots):
          print("snapshots contains testService1")
      else:
          raise Exception("snapshots should contain testService1!")

      # Snapshot everything, modify all state and only restore testService1.
      # Check whether only testService1 has changed.

      coordinator.succeed(
          "${env} disnix-snapshot"
      )

      testtarget1.succeed("echo 5 > /var/db/testService2/state")
      testtarget2.succeed("echo 6 > /var/db/testService1/state")

      coordinator.succeed(
          "${env} disnix-restore --no-upgrade --component testService1"
      )
      result = testtarget1.succeed("cat /var/db/testService2/state")

      if result[:-1] == "5":
          print("testService2 state is 5")
      else:
          raise Exception("testService2 state should be 5, but it is: {}".format(result[:-1]))

      result = testtarget2.succeed("cat /var/db/testService1/state")

      if result[:-1] == "1":
          print("testService1 state is 1")
      else:
          raise Exception("testService1 state should be 1, but it is: {}".format(result[:-1]))

      # Restore all state and clean all snapshots of testService1 only.
      # Check if this is the case.

      coordinator.succeed(
          "${env} disnix-restore --no-upgrade"
      )
      coordinator.succeed(
          "${env} disnix-clean-snapshots --keep 0 --component testService1 ${snapshotTests}/infrastructure.nix"
      )

      result = testtarget2.succeed(
          "dysnomia-snapshots --query-all --component testService1 | wc -l"
      )

      if int(result) == 0:
          print("We have 0 testService1 snapshots left!")
      else:
          raise Exception(
              "We should have 0 testService1 snapshot left, instead we have: {}".format(
                  result
              )
          )

      result = testtarget1.succeed(
          "dysnomia-snapshots --query-all --component testService2 | wc -l"
      )

      if int(result) == 1:
          print("We have 1 testService2 snapshots left!")
      else:
          raise Exception(
              "We should have 1 testService2 snapshot left, instead we have: {}".format(
                  result
              )
          )

      # Reverse deployment again and we run the delete state operation on
      # testService1 only. The end result is that the machine that previously
      # had testService1 deployed has removed its state, while the other
      # machine has the states of both testService1 and testService2.

      coordinator.succeed(
          "${env} disnix-env -s ${snapshotTests}/services-state.nix -i ${snapshotTests}/infrastructure.nix -d ${snapshotTests}/distribution-simple.nix"
      )
      coordinator.succeed(
          "${env} disnix-delete-state --component testService1 /nix/var/nix/profiles/per-user/root/disnix-coordinator/default-2-link"
      )

      testtarget1.succeed("cat /var/db/testService1/state")
      testtarget1.succeed("cat /var/db/testService2/state")
      testtarget2.fail("cat /var/db/testService1/state")
      testtarget2.succeed("cat /var/db/testService2/state")

      # Clean all snapshots on the coordinator and target machines
      coordinator.succeed(
          "${env} dysnomia-snapshots --gc --keep 0"
      )
      coordinator.succeed(
          "${env} disnix-clean-snapshots --keep 0 ${snapshotTests}/infrastructure.nix"
      )

      # Capture snapshots depth-first.
      testtarget1.succeed("echo 1 > /var/db/testService1/state")
      coordinator.succeed(
          "${env} disnix-snapshot --depth-first"
      )

      # Change the state of testService1, restore depth first and check
      # whether it has successfully restored the old state.
      testtarget1.succeed("echo 2 > /var/db/testService1/state")
      coordinator.succeed(
          "${env} disnix-restore --depth-first --no-upgrade"
      )
      result = testtarget1.succeed("cat /var/db/testService1/state")

      if result[:-1] == "1":
          print("testService1 state is: {}".format(result[:-1]))
      else:
          raise Exception(
              "testService1 state should be: 1, instead it is: {}".format(result[:-1])
          )

      # Test disnix-reconstruct. Because nothing has changed the coordinator
      # profile should remain identical.

      oldNumOfGenerations = coordinator.succeed(
          "ls /nix/var/nix/profiles/per-user/root/disnix-coordinator | wc -l"
      )
      coordinator.succeed(
          "${env} disnix-reconstruct ${snapshotTests}/infrastructure.nix"
      )
      newNumOfGenerations = coordinator.succeed(
          "ls /nix/var/nix/profiles/per-user/root/disnix-coordinator | wc -l"
      )

      if int(oldNumOfGenerations) == int(newNumOfGenerations):
          print("The amount of manifest generations remained the same!")
      else:
          raise Exception("The amount of manifest generations should remain the same!")

      # Test disnix-reconstruct. First, we remove the old manifests. They
      # should have been reconstructed.

      coordinator.succeed("rm /nix/var/nix/profiles/per-user/root/disnix-coordinator/*")
      coordinator.succeed(
          "${env} disnix-reconstruct ${snapshotTests}/infrastructure.nix"
      )
      result = coordinator.succeed(
          "ls /nix/var/nix/profiles/per-user/root/disnix-coordinator | wc -l"
      )

      if int(result) == 2:
          print("We have a reconstructed manifest!")
      else:
          raise Exception("We don't have any reconstructed manifests!")

      # We pretend that testTarget2 has disappeared and we redeploy all
      # services to one single machine (testTarget1). Deployment should still
      # succeed as the disappeared machine is not in the infrastructure model
      # anymore.

      coordinator.succeed(
          "${env} disnix-env -s ${snapshotTests}/services-state.nix -i ${snapshotTests}/infrastructure-single.nix -d ${snapshotTests}/distribution-single.nix"
      )
    '';
}
