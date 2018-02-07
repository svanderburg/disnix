{nixpkgs, dysnomia, disnix, disnixRemoteClient}:

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
        env = "NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' DISNIX_REMOTE_CLIENT=${disnixRemoteClient}";
      in
      ''
        startAll;

        my $manifest = $client->mustSucceed("${env} disnix-manifest -s ${manifestTests}/services-composition.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-composition.nix");
        my @closure = split('\n', $client->mustSucceed("nix-store -qR $manifest"));

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

        my $result = $client->mustSucceed("${env} disnix-ssh-client --target server --print-invalid /nix/store/invalid");

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

        my @target2Profile = grep(/\-testtarget2/, @closure);
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
        my @defaultProfileClosure = split('\n', $server->mustSucceed("nix-store -qR /nix/var/nix/profiles/disnix/default"));
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
        my @service = grep(/testService2/, @closure);

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

        @closure = split('\n', $client->mustSucceed("nix-store -qR $manifest"));
        my @testService1 = grep(/\-testService1/, @closure);

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

        # Shell test. We run a shell session in which we create a tempfile,
        # then we check whether the file exists and contains 'foo'
        $client->mustSucceed("${env} disnix-ssh-client --target server --shell --arguments foo=foo --arguments bar=bar --type echo --command 'echo \$foo > /tmp/tmpfile' @testService1 >&2");
        $server->mustSucceed("grep 'foo' /tmp/tmpfile");
      '';
  }
