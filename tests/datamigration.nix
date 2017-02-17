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
        
        # Delete the state of the services. Because none of them have been marked
        # as garbage, they should be kept.
        
        $coordinator->mustSucceed("${env} disnix-delete-state /nix/var/nix/profiles/per-user/root/disnix-coordinator/default-1-link");
        
        $testtarget1->mustSucceed("cat /var/db/testService1/state");
        $testtarget2->mustSucceed("cat /var/db/testService2/state");
        
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
        
        # Clean all snapshots on the target and coordinator machines for the component filtering tests
        $coordinator->mustSucceed("${env} disnix-clean-snapshots ${snapshotTests}/infrastructure.nix --keep 0");
        $coordinator->mustSucceed("${env} dysnomia-snapshots --gc --keep 0");
        
        # Only take a snapshot of testService1 and check whether this is the case
        $coordinator->mustSucceed("${env} disnix-snapshot --component testService1");
        my @snapshots = $coordinator->mustSucceed("${env} dysnomia-snapshots --query-all --container wrapper");
        
        if(scalar @snapshots == 1) {
            print "snapshots length is: 1\n";
        } else {
            die "snapshot length should be: 1!";
        }
        
        my @testService1 = grep(/testService1/, @snapshots);
        
        if(scalar @testService1 == 1) {
            print "snapshots contains testService1\n"
        } else {
            die "snapshots should contain testService1!";
        }
        
        # Snapshot everything, modify all state and only restore testService1.
        # Check whether only testService1 has changed.
        
        $coordinator->mustSucceed("${env} disnix-snapshot");
        
        $testtarget1->mustSucceed("echo 5 > /var/db/testService2/state");
        $testtarget2->mustSucceed("echo 6 > /var/db/testService1/state");
        
        $coordinator->mustSucceed("${env} disnix-restore --no-upgrade --component testService1");
        $result = $testtarget1->mustSucceed("cat /var/db/testService2/state");
        
        if($result == 5) {
            print "testService2 state is 5\n";
        } else {
            die "testService2 state should be 5, but it is: $result";
        }
        
        $result = $testtarget2->mustSucceed("cat /var/db/testService1/state");
        
        if($result == 1) {
            print "testService1 state is 1\n";
        } else {
            die "testService1 state should be 1, but it is: $result";
        }
        
        # Restore all state and clean all snapshots of testService1 only.
        # Check if this is the case.
        
        $coordinator->mustSucceed("${env} disnix-restore --no-upgrade");
        $coordinator->mustSucceed("${env} disnix-clean-snapshots --keep 0 --component testService1 ${snapshotTests}/infrastructure.nix");
        
        $result = $testtarget2->mustSucceed("dysnomia-snapshots --query-all --component testService1 | wc -l");
        
        if($result == 0) {
            print "We have 0 testService1 snapshots left!\n";
        } else {
            die "We should have 0 testService1 snapshot left, instead we have: $result";
        }
        
        $result = $testtarget1->mustSucceed("dysnomia-snapshots --query-all --component testService2 | wc -l");
        
        if($result == 1) {
            print "We have 1 testService2 snapshots left!\n";
        } else {
            die "We should have 1 testService2 snapshot left, instead we have: $result";
        }
        
        # Reverse deployment again and we run the delete state operation on
        # testService1 only. The end result is that the machine that previously
        # had testService1 deployed has removed its state, while the other
        # machine has the states of both testService1 and testService2.
        
        $coordinator->mustSucceed("${env} disnix-env -s ${snapshotTests}/services-state.nix -i ${snapshotTests}/infrastructure.nix -d ${snapshotTests}/distribution-simple.nix --no-delete-state");
        $coordinator->mustSucceed("${env} disnix-delete-state --component testService1 /nix/var/nix/profiles/per-user/root/disnix-coordinator/default-2-link");
        
        $testtarget1->mustSucceed("cat /var/db/testService1/state");
        $testtarget1->mustSucceed("cat /var/db/testService2/state");
        $testtarget2->mustFail("cat /var/db/testService1/state");
        $testtarget2->mustSucceed("cat /var/db/testService2/state");
        
        # Clean all snapshots on the coordinator and target machines 
        $coordinator->mustSucceed("${env} dysnomia-snapshots --gc --keep 0");
        $coordinator->mustSucceed("${env} disnix-clean-snapshots --keep 0 ${snapshotTests}/infrastructure.nix");
        
        # Capture snapshots depth-first.
        $testtarget1->mustSucceed("echo 1 > /var/db/testService1/state");
        $coordinator->mustSucceed("${env} disnix-snapshot --depth-first");
        
        # Change the state of testService1, restore depth first and check
        # whether it has successfully restored the old state.
        $testtarget1->mustSucceed("echo 2 > /var/db/testService1/state");
        $coordinator->mustSucceed("${env} disnix-restore --depth-first --no-upgrade");
        $result = $testtarget1->mustSucceed("cat /var/db/testService1/state");
        
        if($result == 1) {
            print "testService1 state is: $result";
        } else {
            die "testService1 state should be: 1, instead it is: $result";
        }
        
        # Test disnix-reconstruct. Because nothing has changed the coordinator
        # profile should remain identical.
        
        my $oldNumOfGenerations = $coordinator->mustSucceed("ls /nix/var/nix/profiles/per-user/root/disnix-coordinator | wc -l");
        $coordinator->mustSucceed("${env} disnix-reconstruct ${snapshotTests}/infrastructure.nix");
        my $newNumOfGenerations = $coordinator->mustSucceed("ls /nix/var/nix/profiles/per-user/root/disnix-coordinator | wc -l");
        
        if($oldNumOfGenerations == $newNumOfGenerations) {
            print "The amount of manifest generations remained the same!\n";
        } else {
            die "The amount of manifest generations should remain the same!";
        }
        
        # Test disnix-reconstruct. First, we remove the old manifests. They
        # should have been reconstructed.
        
        $coordinator->mustSucceed("rm /nix/var/nix/profiles/per-user/root/disnix-coordinator/*");
        $coordinator->mustSucceed("${env} disnix-reconstruct ${snapshotTests}/infrastructure.nix");
        $result = $coordinator->mustSucceed("ls /nix/var/nix/profiles/per-user/root/disnix-coordinator | wc -l");
        
        if($result == 2) {
            print "We have a reconstructed manifest!\n";
        } else {
            die "We don't have any reconstructed manifests!";
        }
      '';
  }
