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
        env = ''DYSNOMIA_STATEDIR=/root/dysnomia SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' NIX_PATH='nixpkgs=${nixpkgs}' dysnomia=\"\$(dirname \$(readlink -f \$(type -p dysnomia)))/..\"'';
      in
      ''
        startAll;
        
        # Initialise ssh stuff by creating a key pair for communication
        my $key=`${pkgs.openssh}/bin/ssh-keygen -t dsa -f key -N ""`;
        
        $testtarget1->mustSucceed("mkdir -m 700 /root/.ssh");
        $testtarget1->copyFileFromHost("key.pub", "/root/.ssh/authorized_keys");

        $testtarget2->mustSucceed("mkdir -m 700 /root/.ssh");
        $testtarget2->copyFileFromHost("key.pub", "/root/.ssh/authorized_keys");
        
        $coordinator->mustSucceed("mkdir -m 700 /root/.ssh");
        $coordinator->copyFileFromHost("key", "/root/.ssh/id_dsa");
        $coordinator->mustSucceed("chmod 600 /root/.ssh/id_dsa");
        
        # Use disnix-env to perform a new installation.
        # This test should succeed.
        $coordinator->mustSucceed("${env} disnix-env -s ${snapshotTests}/services-state.nix -i ${snapshotTests}/infrastructure.nix -d ${snapshotTests}/distribution-simple.nix");
        
        # Check if both machines have state deployed
        my $result = $testtarget1->mustSucceed("cat /var/db/testService1/state");
        
        if($result == 0) {
            print "result is: 0\n";
        } else {
            die "result should be: 0";
        }
        
        $result = $testtarget2->mustSucceed("cat /var/db/testService2/state");
        
        if($result == 0) {
            print "result is: 0\n";
        } else {
            die "result should be: 0";
        }
        
        # Modify the state of each deployed service
        
        $testtarget1->mustSucceed("echo 1 > /var/db/testService1/state");
        $testtarget2->mustSucceed("echo 2 > /var/db/testService2/state");
        
        # Use disnix-env to reverse the location of the deployed services.
        $coordinator->mustSucceed("${env} disnix-env -s ${snapshotTests}/services-state.nix -i ${snapshotTests}/infrastructure.nix -d ${snapshotTests}/distribution-reverse.nix --no-delete-state");
        
        # Check if the state is reversed
        $result = $testtarget1->mustSucceed("cat /var/db/testService2/state");
        
        if($result == 2) {
            print "result is: 2\n";
        } else {
            die "result should be: 2, but it is: $result";
        }
        
        $result = $testtarget2->mustSucceed("cat /var/db/testService1/state");
        
        if($result == 1) {
            print "result is: 1\n";
        } else {
            die "result should be: 1, but it is: $result";
        }
        
        # Take a snapshot of everything and check if we have
        # the one snapshot of each service
        $coordinator->mustSucceed("rm -R /root/dysnomia");
        $coordinator->mustSucceed("${env} disnix-snapshot");
        
        $result = $coordinator->mustSucceed("${env} dysnomia-snapshots --query-all --container wrapper --component testService1 | wc -l");
        
        if($result == 1) {
            print "result is: 1\n";
        } else {
            die "result should be: 1, but it is: $result";
        }
        
        $result = $coordinator->mustSucceed("${env} dysnomia-snapshots --query-all --container wrapper --component testService2 | wc -l");
        
        if($result == 1) {
            print "result is: 1\n";
        } else {
            die "result should be: 1, but it is: $result";
        }
        
        # Check if old state is present, delete it and check if it is actually
        # removed.
        
        $testtarget1->mustSucceed("cat /var/db/testService1/state");
        $testtarget2->mustSucceed("cat /var/db/testService2/state");
        
        $coordinator->mustSucceed("${env} disnix-delete-state /nix/var/nix/profiles/per-user/root/disnix-coordinator/default-1-link");
        
        $testtarget1->mustFail("cat /var/db/testService1/state");
        $testtarget2->mustFail("cat /var/db/testService2/state");
        
        # Delete all the snapshots on the target machines
        
        $coordinator->mustSucceed("${env} disnix-clean-snapshots ${snapshotTests}/infrastructure.nix --keep 0");
        
        $result = $testtarget1->mustSucceed("dysnomia-snapshots --query-all --container wrapper --component testService2 | wc -l");
        
        if($result == 0) {
            print "result is: 0\n";
        } else {
            die "result should be: 0, but it is: $result";
        }
        
        $result = $testtarget2->mustSucceed("dysnomia-snapshots --query-all --container wrapper --component testService1 | wc -l");
        
        if($result == 0) {
            print "result is: 0\n";
        } else {
            die "result should be: 0, but it is: $result";
        }
        
        # Modify the state, restore the snapshots and check whether they have
        # their old state again.
        
        $testtarget1->mustSucceed("echo 5 > /var/db/testService2/state");
        $testtarget2->mustSucceed("echo 6 > /var/db/testService1/state");
        
        $coordinator->mustSucceed("${env} disnix-restore --no-upgrade");
        
        $result = $testtarget1->mustSucceed("cat /var/db/testService2/state");
        
        if($result == 2) {
            print "result is: 2\n";
        } else {
            die "result should be: 2, but it is: $result";
        }
        
        $result = $testtarget2->mustSucceed("cat /var/db/testService1/state");
        
        if($result == 1) {
            print "result is: 1\n";
        } else {
            die "result should be: 1, but it is: $result";
        }
      '';
  }
