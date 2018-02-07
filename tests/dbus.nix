{nixpkgs, dysnomia, disnix}:

let
  manifestTests = ./manifest;
  machine = import ./machine.nix { inherit dysnomia disnix; };
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

        my $manifest = $client->mustSucceed("${env} disnix-manifest -s ${manifestTests}/services-composition.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-composition.nix");
        my @closure = split('\n', $client->mustSucceed("nix-store -qR $manifest"));
        my @target1Profile = grep(/\-testtarget1/, @closure);

        #### Test disnix-client / disnix-service

        # Check invalid path. We query an invalid path from the service
        # which should return the path we have given.
        # This test should succeed.

        my $result = $client->mustSucceed("disnix-client --print-invalid /nix/store/invalid");

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
      '';
  }
