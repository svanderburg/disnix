{nixpkgs, dysnomia, disnix}:

let
  manifestTests = ./manifest;
  machine = import ./machine.nix { inherit dysnomia disnix; enableMultiUser = false; };
in
with import "${nixpkgs}/nixos/lib/testing.nix" { system = builtins.currentSystem; };

  simpleTest {
    nodes = {
      server = machine;
    };
    testScript =
      let
        env = "NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no'";
      in
      ''
        startAll;

        my $manifest = $server->mustSucceed("${env} disnix-manifest -s ${manifestTests}/services-composition.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-composition.nix");
        my @closure = split('\n', $server->mustSucceed("nix-store -qR $manifest"));
        my @target1Profile = grep(/\-testtarget1/, @closure);

        #### Test disnix-run-activity

        # Check invalid path. We query an invalid path from the service
        # which should return the path we have given.
        # This test should succeed.

        my $result = $server->mustSucceed("disnix-run-activity --print-invalid /nix/store/invalid");

        if($result =~ /\/nix\/store\/invalid/) {
            print "/nix/store/invalid is invalid\n";
        } else {
            die "/nix/store/invalid should be invalid\n";
        }

        # Check invalid path. We query a valid path from the service
        # which should return nothing in this case.
        # This test should succeed.

        $result = $server->mustSucceed("disnix-run-activity --print-invalid ${pkgs.bash}");

        if($result =~ /bash/) {
            die "${pkgs.bash} should not be returned!\n";
        } else {
            print "${pkgs.bash} is valid\n";
        }

        # Query requisites test. Queries the requisites of the bash shell
        # and checks whether it is part of the closure.
        # This test should succeed.

        $result = $server->mustSucceed("disnix-run-activity --query-requisites ${pkgs.bash}");

        if($result =~ /bash/) {
            print "${pkgs.bash} is in the closure\n";
        } else {
            die "${pkgs.bash} should be in the closure!\n";
        }

        # Realise test. First the coreutils derivation file is instantiated,
        # then it is realised. This test should succeed.

        $result = $server->mustSucceed("nix-instantiate ${nixpkgs} -A coreutils");
        $server->mustSucceed("disnix-run-activity --realise $result");

        # Set test. Adds the testtarget1 profile as only derivation into 
        # the Disnix profile. We first set the profile, then we check
        # whether the profile is part of the closure.
        # This test should succeed.

        $server->mustSucceed("disnix-run-activity --set --profile default @target1Profile");
        my @defaultProfileClosure = split('\n', $server->mustSucceed("nix-store -qR /nix/var/nix/profiles/disnix/default"));
        @closure = grep("@target1Profile", @defaultProfileClosure);

        if(scalar @closure > 0) {
            print "@target1Profile is part of the closure\n";
        } else {
            die "@target1Profile should be part of the closure\n";
        }

        # Query installed test. Queries the installed services in the 
        # profile, which has been set in the previous testcase.
        # testService1 should be in there. This test should succeed.

        @closure = split('\n', $server->mustSucceed("disnix-run-activity --query-installed --profile default"));
        my @service = grep(/testService1/, @closure);

        if(scalar @service > 0) {
            print "@service is installed in the default profile\n";
        } else {
            die "@service should be installed in the default profile\n";
        }

        # Collect garbage test. This test should succeed.
        # Testcase disabled, as this is very expensive.
        # $server->mustSucceed("disnix-run-activity --collect-garbage");

        # Export test. Exports the closure of the bash shell.
        # This test should succeed.
        $result = $server->mustSucceed("disnix-run-activity --export ${pkgs.bash}");

        # Import test. Imports the exported closure of the previous test.
        # This test should succeed.
        $server->mustSucceed("disnix-run-activity --import $result");

        # Lock test. This test should succeed.
        $server->mustSucceed("disnix-run-activity --lock");

        # Lock test. This test should fail, since the service instance is already locked
        $server->mustFail("disnix-run-activity --lock");

        # Unlock test. This test should succeed, so that we can release the lock
        $server->mustSucceed("disnix-run-activity --unlock");

        # Unlock test. This test should fail as the lock has already been released
        $server->mustFail("disnix-run-activity --unlock");

        # Use the echo type to activate a service.
        # We use the testService1 service defined in the manifest earlier
        # This test should succeed.

        @closure = split('\n', $server->mustSucceed("nix-store -qR $manifest"));
        my @testService1 = grep(/\-testService1/, @closure);

        $server->mustSucceed("disnix-run-activity --activate --arguments foo=foo --arguments bar=bar --type echo @testService1");

        # Deactivate the same service using the echo type. This test should succeed.
        $server->mustSucceed("disnix-run-activity --deactivate --arguments foo=foo --arguments bar=bar --type echo @testService1");

        # Capture config test. We capture a config and the tempfile should
        # contain one property: "foo" = "bar";
        $result = $server->mustSucceed("disnix-run-activity --capture-config");
        $server->mustSucceed("(cat $result) | grep '\"foo\" = \"bar\"'");
      '';
  }
