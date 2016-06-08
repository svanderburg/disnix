{nixpkgs, dysnomia}:

let
  manifestTests = ./manifest;
  machine = import ./machine.nix { inherit dysnomia; };
in
with import "${nixpkgs}/nixos/lib/testing.nix" { system = builtins.currentSystem; };

  simpleTest {
    nodes = {
      client = machine;
      server = machine;
    };
    testScript =
      let
        env = "NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no'";
      in
      ''
        startAll;
        
        #### Test disnix-instantiate
        
        # Generates a distributed derivation file. The closure should be
        # contain store derivation files. This test should succeed.
        
        my $result = $client->mustSucceed("${env} disnix-instantiate -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix");
        my @closure = split('\n', $client->mustSucceed("nix-store -qR $result"));
        my @derivations = grep(/\.drv/, @closure);
          
        if(scalar(@derivations) > 0) {
            print "The closure of the distributed derivation contains store derivations\n";
        } else {
            die "The closure of the distributed derivation contains no store derivations!\n";
        }
        
        #### Test disnix-manifest
        
        # Complete inter-dependency test. Here we have a services model in 
        # which both the inter-dependency specifications and distribution 
        # is correct and complete. This test should succeed.
        
        $client->mustSucceed("${env} disnix-manifest -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix");
        
        # Incomplete inter-dependency test. Here we have a services model
        # in which an inter-dependency is not specified in the dependsOn
        # attribute. This test should trigger an error.
        
        $client->mustFail("${env} disnix-manifest -s ${manifestTests}/services-incomplete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix");
        
        # Load balancing test. Here we distribute testService1 and
        # testService2 to machines testtarget1 and testtarget2.
        # We verify this by checking whether the services testService1
        # and testService2 are in the closure of the testtarget1 and
        # testtarget2 profiles. This test should succeed.
        
        my $manifest = $client->mustSucceed("${env} disnix-manifest -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-loadbalancing.nix");
        @closure = split('\n', $client->mustSucceed("nix-store -qR $manifest"));
        
        my @target1Profile = grep(/\-testtarget1/, @closure);
        my @target2Profile = grep(/\-testtarget2/, @closure);
        
        my $target1ProfileClosure = $client->mustSucceed("nix-store -qR @target1Profile");
        my $target2ProfileClosure = $client->mustSucceed("nix-store -qR @target2Profile");
        
        if($target1ProfileClosure =~ /\-testService1/) {
            print "testService1 is distributed to testtarget1 -> OK\n";
        } else {
            die "testService1 should be distributed to testtarget1\n";
        }
        
        if($target2ProfileClosure =~ /\-testService1/) {
            print "testService1 is distributed to testtarget2 -> OK\n";
        } else {
            die "testService1 should be distributed to testtarget2\n";
        }

        if($target1ProfileClosure =~ /\-testService2/) {
            print "testService2 is distributed to testtarget1 -> OK\n";
        } else {
            die "testService2 should be distributed to testtarget1\n";
        }
        
        if($target2ProfileClosure =~ /\-testService2/) {
            print "testService2 is distributed to testtarget2 -> OK\n";
        } else {
            die "testService2 should be distributed to testtarget2\n";
        }

        if($target1ProfileClosure =~ /\-testService3/) {
            print "testService3 is distributed to testtarget1 -> OK\n";
        } else {
            die "testService3 should be distributed to testtarget1\n";
        }
        
        if($target2ProfileClosure =~ /\-testService3/) {
            die "testService3 should NOT be distributed to testtarget2\n";
        } else {
            print "testService3 is NOT distributed to testtarget2 -> OK\n";
        }
        
        # Composition test. Here we create a custom inter-dependency by
        # coupling testService3 to testService2B (instead of testService2).
        # We verify this by checking whether this service is the closure
        # of the manifest. This test should succeed.
        
        $manifest = $client->mustSucceed("${env} disnix-manifest -s ${manifestTests}/services-composition.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-composition.nix");
        my $closure = $client->mustSucceed("nix-store -qR $manifest");
        
        if($closure =~ /\-testService2B/) {
            print "Found testService2B";
        } else {
            die "testService2B not found!";
        }
        
        # Incomplete distribution test. Here we have a complete
        # inter-dependency specification in the services model, but we
        # do not distribute the service to any target.
        # This test should trigger an error.
        
        $client->mustFail("${env} disnix-manifest -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-incomplete.nix");
        
        #### Test disnix-client / disnix-service
        
        # Check invalid path. We query an invalid path from the service
        # which should return the path we have given.
        # This test should succeed.
        
        $result = $client->mustSucceed("disnix-client --print-invalid /nix/store/invalid");
        
        if($result =~ /\/nix\/store\/invalid/) {
            print "/nix/store/invalid is invalid\n";
        } else {
            die "/nix/store/invalid should be invalid\n";
        }
        
        # Check invalid path. We query a valid path from the service
        # which should return nothing in this case.
        # This test should succeed.
        
        $result = $client->mustSucceed("disnix-client --print-invalid ${pkgs.bash}");
        
        if($result =~ /bash/) {
            die "${pkgs.bash} should not be returned!\n";
        } else {
            print "${pkgs.bash} is valid\n";
        }
        
        # Query requisites test. Queries the requisites of the bash shell
        # and checks whether it is part of the closure.
        # This test should succeed.
        
        $result = $client->mustSucceed("disnix-client --query-requisites ${pkgs.bash}");
        
        if($result =~ /bash/) {
            print "${pkgs.bash} is in the closure\n";
        } else {
            die "${pkgs.bash} should be in the closure!\n";
        }
        
        # Realise test. First the coreutils derivation file is instantiated,
        # then it is realised. This test should succeed.
        
        $result = $client->mustSucceed("nix-instantiate ${nixpkgs} -A coreutils");
        $client->mustSucceed("disnix-client --realise $result");
        
        # Set test. Adds the testtarget1 profile as only derivation into 
        # the Disnix profile. We first set the profile, then we check
        # whether the profile is part of the closure.
        # This test should succeed.
        
        $client->mustSucceed("disnix-client --set --profile default @target1Profile");
        my @defaultProfileClosure = split('\n', $client->mustSucceed("nix-store -qR /nix/var/nix/profiles/disnix/default"));
        @closure = grep("@target1Profile", @defaultProfileClosure);
        
        if(scalar @closure > 0) {
            print "@target1Profile is part of the closure\n";
        } else {
            die "@target1Profile should be part of the closure\n";
        }
        
        # Query installed test. Queries the installed services in the 
        # profile, which has been set in the previous testcase.
        # testService1 should be in there. This test should succeed.
        
        @closure = split('\n', $client->mustSucceed("disnix-client --query-installed --profile default"));
        my @service = grep(/testService1/, @closure);
        
        if(scalar @service > 0) {
            print "@service is installed in the default profile\n";
        } else {
            die "@service should be installed in the default profile\n";
        }
        
        # Collect garbage test. This test should succeed.
        # Testcase disabled, as this is very expensive.
        # $client->mustSucceed("disnix-client --collect-garbage");
        
        # Export test. Exports the closure of the bash shell.
        # This test should succeed.
        $result = $client->mustSucceed("disnix-client --export ${pkgs.bash}");
        
        # Import test. Imports the exported closure of the previous test.
        # This test should succeed.
        $client->mustSucceed("disnix-client --import $result");
        
        # Lock test. This test should succeed.
        $client->mustSucceed("disnix-client --lock");
        
        # Lock test. This test should fail, since the service instance is already locked
        $client->mustFail("disnix-client --lock");
        
        # Unlock test. This test should succeed, so that we can release the lock
        $client->mustSucceed("disnix-client --unlock");
        
        # Unlock test. This test should fail as the lock has already been released
        $client->mustFail("disnix-client --unlock");
        
        # Use the echo type to activate a service.
        # We use the testService1 service defined in the manifest earlier
        # This test should succeed.
        
        @closure = split('\n', $client->mustSucceed("nix-store -qR $manifest"));
        my @testService1 = grep(/\-testService1/, @closure);
        
        $client->mustSucceed("disnix-client --activate --arguments foo=foo --arguments bar=bar --type echo @testService1");
        
        # Deactivate the same service using the echo type. This test should succeed.
        $client->mustSucceed("disnix-client --deactivate --arguments foo=foo --arguments bar=bar --type echo @testService1");
        
        # Security test. First we try to invoke a Disnix operation by an
        # unprivileged user, which should fail. Then we try the same
        # command by a privileged user, which should succeed.
        
        $client->mustFail("su - unprivileged -c 'disnix-client --print-invalid /nix/store/invalid'");
        $client->mustSucceed("su - privileged -c 'disnix-client --print-invalid /nix/store/invalid'");
        
        # Logfiles test. We perform an operation and check the id of the
        # logfile. Then we stop the Disnix service and start it again and perform
        # another operation. It should create a logfile which id is one higher.
        
        $client->mustSucceed("disnix-client --print-invalid /nix/store/invalid");
        $result = $client->mustSucceed("ls /var/log/disnix | sort -n | tail -1");
        $client->stopJob("disnix");
        $client->startJob("disnix");
        $client->waitForJob("disnix");
        $client->mustSucceed("sleep 3; disnix-client --print-invalid /nix/store/invalid");
        my $result2 = $client->mustSucceed("ls /var/log/disnix | sort -n | tail -1");
        
        if((substr $result2, 0, -1) - (substr $result, 0, -1) == 1) {
            print "The log file numbers are correct!\n";
        } else {
            die "The logfile numbers are incorrect!";
        }
        
        # Capture config test. We capture a config and the tempfile should
        # contain one property: "foo" = "bar";
        $result = $client->mustSucceed("disnix-client --capture-config");
        $client->mustSucceed("(cat $result) | grep '\"foo\" = \"bar\"'");
        
        #### Test disnix-ssh-client
        # Initialise ssh stuff by creating a key pair for communication
        my $key=`${pkgs.openssh}/bin/ssh-keygen -t dsa -f key -N ""`;
    
        $server->mustSucceed("mkdir -m 700 /root/.ssh");
        $server->copyFileFromHost("key.pub", "/root/.ssh/authorized_keys");
        
        $client->mustSucceed("mkdir -m 700 /root/.ssh");
        $client->copyFileFromHost("key", "/root/.ssh/id_dsa");
        $client->mustSucceed("chmod 600 /root/.ssh/id_dsa");
        
        # Check invalid path. We query an invalid path from the service
        # which should return the path we have given.
        # This test should succeed.
        
        $result = $client->mustSucceed("${env} disnix-ssh-client --target server --print-invalid /nix/store/invalid");
        
        if($result =~ /\/nix\/store\/invalid/) {
            print "/nix/store/invalid is invalid\n";
        } else {
            die "/nix/store/invalid should be invalid\n";
        }
        
        # Check invalid path. We query a valid path from the service
        # which should return nothing in this case.
        # This test should succeed.
        
        $result = $client->mustSucceed("${env} disnix-ssh-client --target server --print-invalid ${pkgs.bash}");
        
        # Query requisites test. Queries the requisites of the bash shell
        # and checks whether it is part of the closure.
        # This test should succeed.
        
        $result = $client->mustSucceed("${env} disnix-ssh-client --target server --query-requisites ${pkgs.bash}");
        
        if($result =~ /bash/) {
            print "${pkgs.bash} is in the closure\n";
        } else {
            die "${pkgs.bash} should be in the closure!\n";
        }
        
        # Realise test. First the coreutils derivation file is instantiated,
        # then it is realised. This test should succeed.
        
        $result = $server->mustSucceed("nix-instantiate ${nixpkgs} -A coreutils");
        $client->mustSucceed("${env} disnix-ssh-client --target server --realise $result");
        
        # Export test. Exports the closure of the bash shell on the server
        # and then imports it on the client. This test should succeed.
        
        $result = $client->mustSucceed("${env} disnix-ssh-client --target server --export --remotefile ${pkgs.bash}");
        $client->mustSucceed("nix-store --import < $result");
        
        # Repeat the same export operation, but now as a localfile. It should
        # export the same closure to a file. This test should succeed.
        $result = $client->mustSucceed("${env} disnix-ssh-client --target server --export --localfile ${pkgs.bash}");
        $server->mustSucceed("[ -e $result ]");
        
        # Import test. Creates a closure of the target2Profile on the
        # client. Then it imports the closure into the Nix store of the 
        # server. This test should succeed.
        
        $server->mustFail("nix-store --check-validity @target2Profile");
        $client->mustSucceed("nix-store --export \$(nix-store -qR @target2Profile) > /root/target2Profile.closure");
        $client->mustSucceed("${env} disnix-ssh-client --target server --import --localfile /root/target2Profile.closure");
        $server->mustSucceed("nix-store --check-validity @target2Profile");
        
        # Do a remotefile import. It should import the bash closure stored
        # remotely. This test should succeed.
        $server->mustSucceed("nix-store --export \$(nix-store -qR /bin/sh) > /root/bash.closure");
        $client->mustSucceed("${env} disnix-ssh-client --target server --import --remotefile /root/bash.closure");
        
        # Set test. Adds the testtarget2 profile as only derivation into 
        # the Disnix profile. We first set the profile, then we check
        # whether the profile is part of the closure.
        # This test should succeed.
        
        $client->mustSucceed("${env} disnix-ssh-client --target server --set --profile default @target2Profile");
        @defaultProfileClosure = split('\n', $server->mustSucceed("nix-store -qR /nix/var/nix/profiles/disnix/default"));
        @closure = grep("@target2Profile", @defaultProfileClosure);
        
        if(scalar @closure > 0) {
            print "@target2Profile is part of the closure\n";
        } else {
            die "@target2Profile should be part of the closure\n";
        }
        
        # Query installed test. Queries the installed services in the 
        # profile, which has been set in the previous testcase.
        # testService2 should be in there. This test should succeed.
        
        @closure = split('\n', $client->mustSucceed("${env} disnix-ssh-client --target server --query-installed --profile default"));
        @service = grep(/testService2/, @closure);
        
        if(scalar @service > 0) {
            print "@service is installed in the default profile\n";
        } else {
            die "@service should be installed in the default profile\n";
        }
        
        # Collect garbage test. This test should succeed.
        # Testcase disabled, as this is very expensive.
        # $client->mustSucceed("${env} disnix-ssh-client --target server --collect-garbage");

        # Lock test. This test should succeed.
        $client->mustSucceed("${env} disnix-ssh-client --target server --lock");
          
        # Lock test. This test should fail, since the service instance is already locked
        $client->mustFail("${env} disnix-ssh-client --target server --lock");
        
        # Unlock test. This test should succeed, so that we can release the lock
        $client->mustSucceed("${env} disnix-ssh-client --target server --unlock");
        
        # Unlock test. This test should fail as the lock has already been released
        $client->mustFail("${env} disnix-ssh-client --target server --unlock");
        
        # Use the echo type to activate a service.
        # We use the testService1 service defined in the manifest earlier
        # This test should succeed.
        $client->mustSucceed("${env} disnix-ssh-client --target server --activate --arguments foo=foo --arguments bar=bar --type echo @testService1");
        
        # Deactivate the same service using the echo type. This test should succeed.
        $client->mustSucceed("${env} disnix-ssh-client --target server --deactivate --arguments foo=foo --arguments bar=bar --type echo @testService1");
        
        # Deactivate the same service using the echo type. This test should succeed.
        $client->mustSucceed("${env} disnix-ssh-client --target server --deactivate --arguments foo=foo --arguments bar=bar --type echo @testService1");
        
        # Capture config test. We capture a config and the tempfile should
        # contain one property: "foo" = "bar";
        $client->mustSucceed("${env} disnix-ssh-client --target server --capture-config | grep '\"foo\" = \"bar\"'");
        
        #### Test disnix-copy-closure
        
        # Test copy closure. Here, we first dermine the closure of
        # testService3 and then we copy the closure of testService3 from
        # the client to the server. This test should succeed.
        
        @closure = split('\n', $client->mustSucceed("nix-store -qR $manifest"));
        my @testService3 = grep(/\-testService3/, @closure);
        
        $server->mustFail("nix-store --check-validity @testService3");
        $client->mustSucceed("${env} disnix-copy-closure --target server --to @testService3");
        $server->mustSucceed("nix-store --check-validity @testService3");
        
        # Test copy closure. Here, we first build a package on the server,
        # which is not on the client. Then we copy the package from the
        # server to the client and we check whether the path is valid.
        
        $result = $server->mustSucceed("nix-build ${nixpkgs} -A writeTextFile --argstr name test --argstr text 'Hello world'");
        $client->mustFail("nix-store --check-validity $result");
        $client->mustSucceed("${env} disnix-copy-closure --target server --from $result");
        $client->mustSucceed("nix-store --check-validity $result");
        
        #### Test disnix-gendist-roundrobin
        
        # Run disnix-gendist-roundrobin and check whether we can use the
        # generated distribution model to build the system.
        # This test should succeed.
        
        $result = $client->mustSucceed("${env} disnix-gendist-roundrobin -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix");
        $result = $client->mustSucceed("${env} disnix-manifest -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d $result");
        
        #### Test disnix-visualize
        
        # Makes a visualization of the roundrobin distribution. the output
        # is produced as a Hydra report. This test should succeed.
        
        $client->mustSucceed("(disnix-visualize $result) > visualize.dot");
        $client->mustSucceed("${pkgs.graphviz}/bin/dot -Tpng visualize.dot > /tmp/xchg/visualize.png");
      '';
  }
