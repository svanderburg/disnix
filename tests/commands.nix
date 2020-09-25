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
      env = ''NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' dysnomia=\"$(dirname $(readlink -f $(type -p dysnomia)))/..\"'';
    in
    ''
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

      # Create a distributed derivation file
      distderivation = coordinator.succeed(
          "${env} disnix-instantiate -s ${snapshotTests}/services-state.nix -i ${snapshotTests}/infrastructure.nix -d ${snapshotTests}/distribution-simple.nix"
      )

      # Delegate the builds using the distributed derivation file
      coordinator.succeed(
          "${env} disnix-build {}".format(
              distderivation
          )
      )

      # Checks whether the testService1 has been actually built on the
      # targets by checking the logfiles. This test should succeed.

      testtarget1.succeed('[ "$(cat /var/log/disnix/2 | grep "\\-testService1.drv")" != "" ]')

      # Create a manifest
      manifest = coordinator.succeed(
          "${env} disnix-manifest -s ${snapshotTests}/services-state.nix -i ${snapshotTests}/infrastructure.nix -d ${snapshotTests}/distribution-simple.nix"
      )

      # Distribute closures to the target machines
      coordinator.succeed(
          "${env} disnix-distribute {}".format(
              manifest
          )
      )

      # Lock the machines
      coordinator.succeed(
          "${env} disnix-lock {}".format(
              manifest
          )
      )

      # Try another lock. This should fail, since the machines have been locked already
      coordinator.fail(
          "${env} disnix-lock {}".format(
              manifest
          )
      )

      # Activate the configuration.
      coordinator.succeed(
          "${env} disnix-activate {}".format(
              manifest
          )
      )

      # Migrate the data from one machine to another
      coordinator.succeed(
          "${env} disnix-migrate {}".format(
              manifest
          )
      )

      # Set the new configuration Nix profiles on the coordinator and target machines
      coordinator.succeed(
          "${env} disnix-set {}".format(
              manifest
          )
      )

      result = coordinator.succeed(
          "ls /nix/var/nix/profiles/per-user/root/disnix-coordinator | wc -l"
      )

      if int(result) == 2:
          print("We have only one generation symlink on the coordinator!")
      else:
          raise Exception("We should have one generation symlink on the coordinator!")

      result = testtarget1.succeed("ls /nix/var/nix/profiles/disnix | wc -l")

      if int(result) == 2:
          print("We have only one generation symlink on target1!")
      else:
          raise Exception("We should have one generation symlink on target1!")

      result = testtarget2.succeed("ls /nix/var/nix/profiles/disnix | wc -l")

      if int(result) == 2:
          print("We have only one generation symlink on target2!")
      else:
          raise Exception("We should have one generation symlink on target2!")

      # Unlock the machines
      coordinator.succeed(
          "${env} disnix-lock -u {}".format(
              manifest
          )
      )

      # Try to unlock the machines again. This should fail, since the machines have been unlocked already
      coordinator.fail(
          "${env} disnix-lock -u {}".format(
              manifest
          )
      )
    '';
}
