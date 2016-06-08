{nixpkgs, dysnomia}:

let
  manifestTests = ./manifest;
  machine = import ./machine.nix { inherit dysnomia; };
in
with import "${nixpkgs}/nixos/lib/testing.nix" { system = builtins.currentSystem; };

  simpleTest {
    nodes = {
      coordinator = machine;
      testtarget1 = machine;
      testtarget2 = machine;
    };
    testScript =
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
        
        # Do a rollback. Since there is nothing deployed, it should fail.
        $coordinator->mustFail("NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-env --rollback");
        
        # Use disnix-env to perform a new installation that fails.
        # It should properly do a rollback.
        $coordinator->mustFail("NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-env -s ${manifestTests}/services-fail.nix -i ${manifestTests}/infrastructure-single.nix -d ${manifestTests}/distribution-fail.nix");
        
        # Use disnix-env to perform a new installation.
        # This test should succeed.
        $coordinator->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-env -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix");
        
        # Do another rollback. Since there is no previous deployment, it should fail.
        $coordinator->mustFail("NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-env --rollback");
        
        # Use disnix-query to see if the right services are installed on
        # the right target platforms. This test should succeed.
        
        my @lines = split('\n', $coordinator->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-query ${manifestTests}/infrastructure.nix"));
        
        if($lines[1] ne "Services on: testtarget1") {
            die "disnix-query output line 1 does not match what we expect!\n";
        }
        
        if($lines[3] =~ /\-testService1/) {
            print "Found testService1 on disnix-query output line 3\n";
        } else {
            die "disnix-query output line 3 does not contain testService1!\n";
        }
        
        if($lines[5] ne "Services on: testtarget2") {
            die "disnix-query output line 5 does not match what we expect!\n";
        }
        
        if($lines[7] =~ /\-testService2/) {
            print "Found testService2 on disnix-query output line 7\n";
        } else {
            die "disnix-query output line 7 does not contain testService2!\n";
        }
        
        if($lines[8] =~ /\-testService3/) {
            print "Found testService3 on disnix-query output line 8\n";
        } else {
            die "disnix-query output line 8 does not contain testService3!\n";
        }
        
        # Check the disnix logfiles to see whether it has indeed activated
        # the services in the distribution model. This test should
        # succeed.
        
        $testtarget1->mustSucceed("[ \"\$(cat /var/log/disnix/8 | grep \"activate: $lines[3]\")\" != \"\" ]");
        $testtarget2->mustSucceed("[ \"\$(cat /var/log/disnix/3 | grep \"activate: $lines[7]\")\" != \"\" ]");
        $testtarget2->mustSucceed("[ \"\$(cat /var/log/disnix/4 | grep \"activate: $lines[8]\")\" != \"\" ]");
        
        # Check if there is only one generation link in the coordinator profile
        # folder and one generation link in the target profiles folder on each
        # machine.
        
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
        
        # We repeat the previous disnix-env command. No changes should be
        # performed. Moreover, we should still have one coordinator profile
        # and one target profile per machine. This test should succeed.
        
        $coordinator->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-env -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix");
        
        $result = $coordinator->mustSucceed("ls /nix/var/nix/profiles/per-user/root/disnix-coordinator | wc -l");
        
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
        
        # We now perform an upgrade by moving testService2 to another machine.
        # This test should succeed.
        $coordinator->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-env -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-reverse.nix");
        
        @lines = split('\n', $coordinator->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-query ${manifestTests}/infrastructure.nix"));
        
        if($lines[3] =~ /\-testService1/) {
            print "Found testService1 on disnix-query output line 3\n";
        } else {
            die "disnix-query output line 3 does not contain testService1!\n";
        }
        
        if($lines[4] =~ /\-testService2/) {
            print "Found testService2 on disnix-query output line 4\n";
        } else {
            die "disnix-query output line 4 does not contain testService2!\n";
        }
        
        if($lines[8] =~ /\-testService3/) {
            print "Found testService3 on disnix-query output line 8\n";
        } else {
            die "disnix-query output line 8 does not contain testService3!\n";
        }
        
        # Since the deployment state has changes, we should now have a new
        # profile entry added on the coordinator and target machines.
        $result = $coordinator->mustSucceed("ls /nix/var/nix/profiles/per-user/root/disnix-coordinator | wc -l");
        
        if($result == 3) {
            print "We have two generation symlinks on the coordinator!\n";
        } else {
            die "We should have two generation symlinks on the coordinator!";
        }
        
        $result = $testtarget1->mustSucceed("ls /nix/var/nix/profiles/disnix | wc -l");
        
        if($result == 3) {
            print "We have two generation symlinks on target1!\n";
        } else {
            die "We should have two generation symlinks on target1!";
        }
        
        $result = $testtarget2->mustSucceed("ls /nix/var/nix/profiles/disnix | wc -l");
        
        if($result == 3) {
            print "We have two generation symlinks on target2!\n";
        } else {
            die "We should have two generation symlinks on target2!";
        }
        
        # Now we undo the upgrade again by moving testService2 back.
        # This test should succeed.
        $coordinator->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-env -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix");
        
        @lines = split('\n', $coordinator->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-query ${manifestTests}/infrastructure.nix"));
        
        if($lines[3] =~ /\-testService1/) {
            print "Found testService1 on disnix-query output line 3\n";
        } else {
            die "disnix-query output line 3 does not contain testService1!\n";
        }
        
        if($lines[7] =~ /\-testService2/) {
            print "Found testService2 on disnix-query output line 7\n";
        } else {
            die "disnix-query output line 7 does not contain testService2!\n";
        }
        
        if($lines[8] =~ /\-testService3/) {
            print "Found testService3 on disnix-query output line 8\n";
        } else {
            die "disnix-query output line 8 does not contain testService3!\n";
        }
        
        # We now perform an upgrade. In this case testService2 is replaced
        # by testService2B. This test should succeed.
        
        $coordinator->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-env -s ${manifestTests}/services-composition.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-composition.nix");
        
        # Use disnix-query to see if the right services are installed on
        # the right target platforms. This test should succeed.
        
        @lines = split('\n', $coordinator->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-query ${manifestTests}/infrastructure.nix"));
        
        if($lines[1] ne "Services on: testtarget1") {
            die "disnix-query output line 1 does not match what we expect!\n";
        }
        
        if($lines[3] =~ /\-testService1/) {
            print "Found testService1 on disnix-query output line 3\n";
        } else {
            die "disnix-query output line 3 does not contain testService1!\n";
        }
        
        if($lines[5] ne "Services on: testtarget2") {
            die "disnix-query output line 5 does not match what we expect!\n";
        }
        
        if($lines[7] =~ /\-testService2B/) {
            print "Found testService2B on disnix-query output line 7\n";
        } else {
            die "disnix-query output line 7 does not contain testService2B!\n";
        }
        
        if($lines[8] =~ /\-testService3/) {
            print "Found testService3 on disnix-query output line 8\n";
        } else {
            die "disnix-query output line 8 does not contain testService3!\n";
        }
        
        # We now perform another upgrade. We move all services from
        # testTarget2 to testTarget1. In this case testTarget2 has become
        # unavailable (not defined in infrastructure model), so the service
        # should not be deactivated on testTarget2. This test should
        # succeed.
        
        $coordinator->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-env -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure-single.nix -d ${manifestTests}/distribution-single.nix > result");
        $coordinator->mustSucceed("[ \"\$(grep \"Skip deactivation\" result | grep \"testService2B\" | grep \"testtarget2\")\" != \"\" ]");
        $coordinator->mustSucceed("[ \"\$(grep \"Skip deactivation\" result | grep \"testService3\" | grep \"testtarget2\")\" != \"\" ]");
        
        # Use disnix-query to check whether testService{1,2,3} are
        # available on testtarget1 and testService{2B,3} are still
        # deployed on testtarget2. This test should succeed.
        
        @lines = split('\n', $coordinator->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-query ${manifestTests}/infrastructure.nix"));
        
        if($lines[1] ne "Services on: testtarget1") {
            die "disnix-query output line 1 does not match what we expect!\n";
        }
        
        if($lines[3] =~ /\-testService1/) {
            print "Found testService1 on disnix-query output line 3\n";
        } else {
            die "disnix-query output line 3 does not contain testService1!\n";
        }
        
        if($lines[4] =~ /\-testService2/) {
            print "Found testService1 on disnix-query output line 4\n";
        } else {
            die "disnix-query output line 4 does not contain testService1!\n";
        }
        
        if($lines[5] =~ /\-testService3/) {
            print "Found testService1 on disnix-query output line 5\n";
        } else {
            die "disnix-query output line 5 does not contain testService1!\n";
        }
        
        if($lines[7] ne "Services on: testtarget2") {
            die "disnix-query output line 7 does not match what we expect!\n";
        }
        
        if($lines[9] =~ /\-testService2B/) {
            print "Found testService2B on disnix-query output line 9\n";
        } else {
            die "disnix-query output line 9 does not contain testService1!\n";
        }
        
        if($lines[10] =~ /\-testService3/) {
            print "Found testService3 on disnix-query output line 10\n";
        } else {
            die "disnix-query output line 10 does not contain testService1!\n";
        }
        
        # Do an upgrade with a transitive dependency. In this test we change the
        # binding of testService3 to a testService2 instance that changes its
        # interdependency from testService1 to testService1B. As a result, both
        # testService2 and testService3 must be redeployed.
        # This test should succeed.
        
        $coordinator->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-env -s ${manifestTests}/services-transitivecomposition.nix -i ${manifestTests}/infrastructure-single.nix -d ${manifestTests}/distribution-transitivecomposition.nix > result");
        
        # Use disnix-query to check whether testService{1,1B,2,3} are
        # available on testtarget1. This test should succeed.
        
        @lines = split('\n', $coordinator->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-query ${manifestTests}/infrastructure.nix"));
        
        if($lines[1] ne "Services on: testtarget1") {
            die "disnix-query output line 1 does not match what we expect!\n";
        }
        
        if($lines[3] =~ /\-testService1/) {
            print "Found testService1 on disnix-query output line 3\n";
        } else {
            die "disnix-query output line 3 does not contain testService1!\n";
        }
        
        if($lines[4] =~ /\-testService1B/) {
            print "Found testService1B on disnix-query output line 4\n";
        } else {
            die "disnix-query output line 3 does not contain testService1B!\n";
        }
        
        if($lines[5] =~ /\-testService2/) {
            print "Found testService1 on disnix-query output line 5\n";
        } else {
            die "disnix-query output line 5 does not contain testService1!\n";
        }
        
        if($lines[6] =~ /\-testService3/) {
            print "Found testService1 on disnix-query output line 6\n";
        } else {
            die "disnix-query output line 6 does not contain testService1!\n";
        }
        
        # Do an upgrade to an environment containing only one service that's a running process.
        # This test should succeed.
        $coordinator->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-env -s ${manifestTests}/services-echo.nix -i ${manifestTests}/infrastructure-single.nix -d ${manifestTests}/distribution-process.nix > result");
        
        # Do a type upgrade. We change the type of the process from 'echo' to
        # 'wrapper', triggering a redeployment. This test should succeed.
        $coordinator->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-env -s ${manifestTests}/services-process.nix -i ${manifestTests}/infrastructure-single.nix -d ${manifestTests}/distribution-process.nix > result");
        
        # Check if the 'process' has written the tmp file.
        # This test should succeed.
        $testtarget1->mustSucceed("sleep 10 && [ -f /tmp/process_out ] && rm /tmp/process_out");
        
        # Do an upgrade that intentionally fails.
        # This test should fail.
        $coordinator->mustFail("NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-env -s ${manifestTests}/services-fail.nix -i ${manifestTests}/infrastructure-single.nix -d ${manifestTests}/distribution-fail.nix > result");
        
        # Check if the 'process' has written the tmp file again.
        # This test should succeed.
        $testtarget1->mustSucceed("sleep 10 && [ -f /tmp/process_out ] && rm /tmp/process_out");
        
        # Roll back to the previously deployed configuration
        $coordinator->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-env --rollback");
        
        # We should have one service of type echo now on the testtarget1 machine
        $testtarget1->mustSucceed("[ \"\$(cat /nix/var/nix/profiles/disnix/default/manifest | tail -1)\" = \"echo\" ]");
        
        # Roll back to the first deployed configuration
        $coordinator->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-env --switch-to-generation 1");
        
        # Use disnix-query to see if the right services are installed on
        # the right target platforms. This test should succeed.
        
        @lines = split('\n', $coordinator->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-query ${manifestTests}/infrastructure.nix"));
        
        if($lines[1] ne "Services on: testtarget1") {
            die "disnix-query output line 1 does not match what we expect!\n";
        }
        
        if($lines[3] =~ /\-testService1/) {
            print "Found testService1 on disnix-query output line 3\n";
        } else {
            die "disnix-query output line 3 does not contain testService1!\n";
        }
        
        if($lines[5] ne "Services on: testtarget2") {
            die "disnix-query output line 5 does not match what we expect!\n";
        }
        
        if($lines[7] =~ /\-testService2/) {
            print "Found testService2 on disnix-query output line 7\n";
        } else {
            die "disnix-query output line 7 does not contain testService2!\n";
        }
        
        if($lines[8] =~ /\-testService3/) {
            print "Found testService3 on disnix-query output line 8\n";
        } else {
            die "disnix-query output line 8 does not contain testService3!\n";
        }
        
        # Test the alternative and more verbose distribution, which does
        # the same thing as the simple distribution.
        $coordinator->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-env -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-alternate.nix");
        
        # Test multi container deployment.
        $coordinator->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-env -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-multicontainer.nix");
        
        # Use disnix-query to see if the right services are installed on
        # the right target platforms. This test should succeed.
        
        my @lines = split('\n', $coordinator->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-query ${manifestTests}/infrastructure.nix"));
        
        if($lines[1] ne "Services on: testtarget1") {
            die "disnix-query output line 1 does not match what we expect!\n";
        }
        
        if($lines[3] =~ /\-testService1/) {
            print "Found testService1 on disnix-query output line 3\n";
        } else {
            die "disnix-query output line 3 does not contain testService1!\n";
        }
        
        if($lines[4] =~ /\-testService3/) {
            print "Found testService3 on disnix-query output line 4\n";
        } else {
            die "disnix-query output line 4 does not contain testService3!\n";
        }
        
        if($lines[6] ne "Services on: testtarget2") {
            die "disnix-query output line 6 does not match what we expect!\n";
        }
        
        if($lines[8] =~ /\-testService2/) {
            print "Found testService2 on disnix-query output line 8\n";
        } else {
            die "disnix-query output line 8 does not contain testService2!\n";
        }
        
        if($lines[9] =~ /\-testService3/) {
            print "Found testService3 on disnix-query output line 9\n";
        } else {
            die "disnix-query output line 9 does not contain testService3!\n";
        }
        
        # Test disnix-capture-infra. Capture the container properties of all
        # machines and generate an infrastructure expression from it. It should
        # contain: "foo" = "bar"; twice.
        $result = $coordinator->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-capture-infra ${manifestTests}/infrastructure.nix | grep '\"foo\" = \"bar\"' | wc -l");
        
        if($result == 2) {
           print "We have foo=bar twice in the infrastructure model!\n";
        } else {
            die "We should have foo=bar twice in the infrastructure model. Instead, we have: $result";
        }
        
        # It should also provide a list of supported types twice.
        $result = $coordinator->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-capture-infra ${manifestTests}/infrastructure.nix | grep '\"supportedTypes\" = \\[ \"process\"' | wc -l");
        
        if($result == 2) {
           print "We have supportedTypes twice in the infrastructure model!\n";
        } else {
            die "We should have supportedTypes twice in the infrastructure model. Instead, we have: $result";
        }
      '';
  }
