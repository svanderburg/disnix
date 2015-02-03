{nixpkgs, dysnomia, disnix}:

let
  manifestTests = ./manifest;
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
        $coordinator->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-env -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix");
        
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
        
        $testtarget1->mustSucceed("[ \"\$(journalctl --no-pager --full _SYSTEMD_UNIT=disnix.service | grep \"Activate: $lines[3]\")\" != \"\" ]");
        $testtarget2->mustSucceed("[ \"\$(journalctl --no-pager --full _SYSTEMD_UNIT=disnix.service | grep \"Activate: $lines[7]\")\" != \"\" ]");
        $testtarget2->mustSucceed("[ \"\$(journalctl --no-pager --full _SYSTEMD_UNIT=disnix.service | grep \"Activate: $lines[8]\")\" != \"\" ]");
        
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
      '';
  }
