{nixpkgs, dysnomia, disnix}:

let
  snapshotTests = ./snapshots;
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
      env = ''NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' dysnomia=\"\$(dirname \$(readlink -f \$(type -p dysnomia)))/..\"'';
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

      # Create a distributed derivation file
      my $distderivation = $coordinator->mustSucceed("${env} disnix-instantiate -s ${snapshotTests}/services-state.nix -i ${snapshotTests}/infrastructure.nix -d ${snapshotTests}/distribution-simple.nix");

      # Delegate the builds using the distributed derivation file
      $coordinator->mustSucceed("${env} disnix-build $distderivation");

      # Checks whether the testService1 has been actually built on the
      # targets by checking the logfiles. This test should succeed.

      $testtarget1->mustSucceed("cat /var/log/disnix/2 >&2");
      $testtarget1->mustSucceed("[ \"\$(cat /var/log/disnix/2 | grep \"\\-testService1.drv\")\" != \"\" ]");

      # Create a manifest
      my $manifest = $coordinator->mustSucceed("${env} disnix-manifest -s ${snapshotTests}/services-state.nix -i ${snapshotTests}/infrastructure.nix -d ${snapshotTests}/distribution-simple.nix");

      # Distribute closures to the target machines
      $coordinator->mustSucceed("${env} disnix-distribute $manifest");

      # Lock the machines
      $coordinator->mustSucceed("${env} disnix-lock $manifest");

      # Try another lock. This should fail, since the machines have been locked already
      $coordinator->mustFail("${env} disnix-lock $manifest");

      # Activate the configuration.
      $coordinator->mustSucceed("${env} disnix-activate $manifest");

      # Migrate the data from one machine to another
      $coordinator->mustSucceed("${env} disnix-migrate $manifest");

      # Set the new configuration Nix profiles on the coordinator and target machines
      $coordinator->mustSucceed("${env} disnix-set $manifest");

      my $result = $coordinator->mustSucceed("ls /nix/var/nix/profiles/per-user/root/disnix-coordinator | wc -l");

      if($result == 2) {
          print "We have only one generation symlink on the coordinator!\n";
      } else {
          die "We should have one generation symlink on the coordinator!";
      }

      $result = $testtarget1->mustSucceed("ls /nix/var/nix/profiles/disnix | wc -l");

      if($result == 2) {
          print "We have only one generation symlink on target1!\n";
      } else {
          die "We should have one generation symlink on target1!";
      }

      $result = $testtarget2->mustSucceed("ls /nix/var/nix/profiles/disnix | wc -l");

      if($result == 2) {
          print "We have only one generation symlink on target2!\n";
      } else {
          die "We should have one generation symlink on target2!";
      }

      # Unlock the machines
      $coordinator->mustSucceed("${env} disnix-lock -u $manifest");

      # Try to unlock the machines again. This should fail, since the machines have been unlocked already
      $coordinator->mustFail("${env} disnix-lock -u $manifest");
    '';
}
