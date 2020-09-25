{nixpkgs, dysnomia, disnix}:

let
  lockingTests = ./locking;
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

      # Force deployed services to always yield 0 as exit status for the lock operations.
      testtarget1.succeed("echo -n 0 > /tmp/lock_status")
      testtarget2.succeed("echo -n 0 > /tmp/lock_status")

      # Use disnix-env to perform a new installation.
      # This test should succeed.
      coordinator.succeed(
          "${env} disnix-env -s ${lockingTests}/services.nix -i ${lockingTests}/infrastructure.nix -d ${lockingTests}/distribution-testtarget1.nix"
      )

      # Attempt to acquire a lock. This test should succeed.
      coordinator.succeed(
          "${env} disnix-lock"
      )

      # Attempt to acquire a lock again. This fails because it has been acquired already.
      coordinator.fail(
          "${env} disnix-lock"
      )

      # Release the locks. This test should succeed.
      coordinator.succeed(
          "${env} disnix-lock -u"
      )

      # Release the locks again. This fails, since the locks have already been released.
      coordinator.fail(
          "${env} disnix-lock -u"
      )

      # Force the lock operation to fail (return exit status 1)
      testtarget1.succeed("echo -n 1 > /tmp/lock_status")

      # Attempt to acquire locks. This test should fail because of the lock status being set to 1
      coordinator.fail(
          "${env} disnix-lock"
      )

      # Attempt to upgrade. This fails because of the lock status is forced to yield an exit status of 1
      coordinator.fail(
          "${env} disnix-env -s ${lockingTests}/services.nix -i ${lockingTests}/infrastructure.nix -d ${lockingTests}/distribution-testtarget2.nix"
      )

      # Force an upgrade by disabling locking. This test should succeed.
      coordinator.succeed(
          "${env} disnix-env -s ${lockingTests}/services.nix -i ${lockingTests}/infrastructure.nix -d ${lockingTests}/distribution-testtarget2.nix --no-lock"
      )
    '';
}
