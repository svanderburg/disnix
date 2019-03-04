{nixpkgs, dysnomia, disnix}:

let
  lockingTests = ./locking;
  machine = import ./machine.nix { inherit dysnomia disnix; };
in
with import "${nixpkgs}/nixos/lib/testing.nix" { system = builtins.currentSystem; };

simpleTest {
  nodes = {
    coordinator = machine;
    testtarget1 = machine;
    testtarget2 = machine;
  };
  testScript =
    let
      env = ''DYSNOMIA_STATEDIR=/root/dysnomia SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' NIX_PATH='nixpkgs=${nixpkgs}' dysnomia=\"\$(dirname \$(readlink -f \$(type -p dysnomia)))/..\"'';
    in
    ''
      startAll;

      # Initialise ssh stuff by creating a key pair for communication
      my $key=`${pkgs.openssh}/bin/ssh-keygen -t ecdsa -f key -N ""`;

      $testtarget1->mustSucceed("mkdir -m 700 /root/.ssh");
      $testtarget1->copyFileFromHost("key.pub", "/root/.ssh/authorized_keys");

      $testtarget2->mustSucceed("mkdir -m 700 /root/.ssh");
      $testtarget2->copyFileFromHost("key.pub", "/root/.ssh/authorized_keys");

      $coordinator->mustSucceed("mkdir -m 700 /root/.ssh");
      $coordinator->copyFileFromHost("key", "/root/.ssh/id_dsa");
      $coordinator->mustSucceed("chmod 600 /root/.ssh/id_dsa");

      # Force deployed services to always yield 0 as exit status for the lock operations..
      $testtarget1->mustSucceed("echo -n 0 > /tmp/lock_status");
      $testtarget2->mustSucceed("echo -n 0 > /tmp/lock_status");

      # Use disnix-env to perform a new installation.
      # This test should succeed.
      $coordinator->mustSucceed("${env} disnix-env -s ${lockingTests}/services.nix -i ${lockingTests}/infrastructure.nix -d ${lockingTests}/distribution-testtarget1.nix");

      # Attempt to acquire a lock. This test should succeed.
      $coordinator->mustSucceed("${env} disnix-lock");

      # Attempt to acquire a lock again. This fails because it has been acquired already.
      $coordinator->mustFail("${env} disnix-lock");

      # Release the locks. This test should succeed.
      $coordinator->mustSucceed("${env} disnix-lock -u");

      # Release the locks again. This fails, since the locks have already been released.
      $coordinator->mustFail("${env} disnix-lock -u");

      # Force the lock operation to fail (return exit status 1)
      $testtarget1->mustSucceed("echo -n 1 > /tmp/lock_status");

      # Attempt to acquire locks. This test should fail because of the lock status being set to 1
      $coordinator->mustFail("${env} disnix-lock");

      # Attempt to upgrade. This fails because of the lock status is forced to yield an exit status of 1
      $coordinator->mustFail("${env} disnix-env -s ${lockingTests}/services.nix -i ${lockingTests}/infrastructure.nix -d ${lockingTests}/distribution-testtarget2.nix");

      # Force an upgrade by disabling locking. This test should succeed.
      $coordinator->mustSucceed("${env} disnix-env -s ${lockingTests}/services.nix -i ${lockingTests}/infrastructure.nix -d ${lockingTests}/distribution-testtarget2.nix --no-lock");
    '';
}
