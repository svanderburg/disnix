{ nixpkgs ? <nixpkgs> }:

let
  jobs = rec {
    tarball =
      { disnix ? {outPath = ./.; rev = 1234;}
      , officialRelease ? false
      }:

      with import nixpkgs {};

      releaseTools.sourceTarball {
        name = "disnix-tarball";
        version = builtins.readFile ./version;
        src = disnix;
        inherit officialRelease;

        buildInputs = [ pkgconfig dbus_glib libxml2 libxslt getopt nixUnstable dblatex tetex doxygen nukeReferences ];
        
        # Add documentation in the tarball
        configureFlags = ''
          --with-docbook-rng=${docbook5}/xml/rng/docbook
          --with-docbook-xsl=${docbook5_xsl}/xml/xsl/docbook
        '';
        
        preConfigure = ''
          # TeX needs a writable font cache.
          export VARTEXFONTS=$TMPDIR/texfonts
        '';
        
        preDist = ''
          make -C doc/manual install prefix=$out
          
          make -C doc/manual index.pdf prefix=$out
          cp doc/manual/index.pdf $out/index.pdf
          
          make -C src apidox
          cp -av doc/apidox $out/share/doc/disnix

          # The PDF containes filenames of included graphics (see
          # http://www.tug.org/pipermail/pdftex/2007-August/007290.html).
          # This causes a retained dependency on dblatex, which Hydra
          # doesn't like (the output of the tarball job is distributed
          # to Windows and Macs, so there should be no Linux binaries
          # in the closure).
          nuke-refs $out/index.pdf
          
          echo "doc-pdf manual $out/index.pdf" >> $out/nix-support/hydra-build-products
          echo "doc manual $out/share/doc/disnix/manual" >> $out/nix-support/hydra-build-products
          echo "doc api $out/share/doc/disnix/apidox/html" >> $out/nix-support/hydra-build-products
        '';
      };

    build =
      { tarball ? jobs.tarball {}
      , system ? builtins.currentSystem
      }:

      with import nixpkgs { inherit system; };

      releaseTools.nixBuild {
        name = "disnix";
        src = tarball;

        buildInputs = [ pkgconfig dbus_glib libxml2 libxslt getopt nixUnstable ]
                      ++ lib.optional (!stdenv.isLinux) libiconv
                      ++ lib.optional (!stdenv.isLinux) gettext;
      };
      
    tests = 
      { nixos ? <nixos>
      , disnix_activation_scripts ? (import ../../disnix-activation-scripts/trunk/release.nix {}).build {}
      }:
      
      let
        disnix = build { system = "x86_64-linux"; };
        manifestTests = ./tests/manifest;
        machine =
          {config, pkgs, ...}:
            
          {
            virtualisation.writableStore = true;
            
            ids.gids = { disnix = 200; };
            users.extraGroups = [ { gid = 200; name = "disnix"; } ];
            
            users.extraUsers = [
              { name = "unprivileged";
                group = "users";
                shell = "/bin/sh";
                description = "Unprivileged user for the disnix-service";
              }
              
              { name = "privileged";
                group = "users";
                shell = "/bin/sh";
                extraGroups = [ "disnix" ];
                description = "Privileged user for the disnix-service";
              }
            ];

            services.dbus.enable = true;
            services.dbus.packages = [ disnix ];
            services.openssh.enable = true;
            
            jobs.disnix =
              { description = "Disnix server";

                startOn = "started dbus";

                script =
                  ''
                    export PATH=/var/run/current-system/sw/bin:/var/run/current-system/sw/sbin
                    export HOME=/root
                    
                    ${disnix}/bin/disnix-service --activation-modules-dir=${disnix_activation_scripts}/libexec/disnix/activation-scripts
                  '';
               };
              
            environment.systemPackages = [ pkgs.stdenv disnix ];
          };
      in
      with import "${nixos}/lib/testing.nix" { system = "x86_64-linux"; };
      
      {
        install = simpleTest {
          nodes = {
            client = machine;
            server = machine;
          };
          testScript = 
            ''
              startAll;
              
              #### Test disnix-instantiate
              
              # Generates a distributed derivation file. The closure should be
              # contain store derivation files. This test should succeed.
              
              my $result = $client->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' disnix-instantiate -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix");
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
              
              $client->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' disnix-manifest -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix");
              
              # Incomplete inter-dependency test. Here we have a services model
              # in which an inter-dependency is not specified in the dependsOn
              # attribute. This test should trigger an error.
              
              $client->mustFail("NIX_PATH='nixpkgs=${nixpkgs}' disnix-manifest -s ${manifestTests}/services-incomplete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix");
                              
              # Load balancing test. Here we distribute testService1 and
              # testService2 to machines testtarget1 and testtarget2.
              # We verify this by checking whether the services testService1
              # and testService2 are in the closure of the testtarget1 and
              # testtarget2 profiles. This test should succeed.
              
              my $manifest = $client->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' disnix-manifest -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-loadbalancing.nix");
              my @closure = split('\n', $client->mustSucceed("nix-store -qR $manifest"));
              
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
              
              my $manifest = $client->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' disnix-manifest -s ${manifestTests}/services-composition.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-composition.nix");
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
              
              $client->mustFail("NIX_PATH='nixpkgs=${nixpkgs}' disnix-manifest -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-incomplete.nix");
              
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
              
              my $result = $client->mustSucceed("disnix-client --print-invalid ${pkgs.bash}");
              
              if($result =~ /bash/) {
                  die "${pkgs.bash} should not be returned!\n";
              } else {
                  print "${pkgs.bash} is valid\n";
              }
              
              # Query requisites test. Queries the requisites of the bash shell
              # and checks whether it is part of the closure.
              # This test should succeed.
              
              my $result = $client->mustSucceed("disnix-client --query-requisites ${pkgs.bash}");
              
              if($result =~ /bash/) {
                  print "${pkgs.bash} is in the closure\n";
              } else {
                  die "${pkgs.bash} should be in the closure!\n";
              }
              
              # Realise test. First a bash derivation file is instantiated,
              # then it is realised. This test should succeed.
              
              my $result = $client->mustSucceed("nix-instantiate ${nixpkgs} -A bash");
              $client->mustSucceed("disnix-client --realise $result");
              
              # Set test. Adds the testtarget1 profile as only derivation into 
              # the Disnix profile. We first set the profile, then we check
              # whether the profile is part of the closure.
              # This test should succeed.
              
              $client->mustSucceed("disnix-client --set --profile default @target1Profile");
              my @defaultProfileClosure = split('\n', $client->mustSucceed("nix-store -qR /nix/var/nix/profiles/disnix/default"));
              my @closure = grep("@target1Profile", @defaultProfileClosure);
              
              if("@closure" == "") {
                  print "@target1Profile is part of the closure\n";
              } else {
                  die "@target1Profile should be part of the closure\n";
              }
              
              # Query installed test. Queries the installed services in the 
              # profile, which has been set in the previous testcase.
              # testService1 should be in there. This test should succeed.
              
              my @closure = split('\n', $client->mustSucceed("disnix-client --query-installed --profile default"));
              my @service = grep(/testService1/, @closure);
              
              if("@service" == "") {
                  print "@service is installed in the default profile\n";
              } else {
                  die "@service should be installed in the default profile\n";
              }
              
              # Collect garbage test. This test should succeed.
              # Testcase disabled, as this is very expensive.
              # $client->mustSucceed("disnix-client --collect-garbage");
              
              # Export test. Exports the closure of the bash shell.
              # This test should succeed.
              my $result = $client->mustSucceed("disnix-client --export ${pkgs.bash}");
              
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
              
              my @closure = split('\n', $client->mustSucceed("nix-store -qR $manifest"));
              my @testService1 = grep(/\-testService1/, @closure);
              
              $client->mustSucceed("disnix-client --activate --arguments foo=foo --arguments bar=bar --type echo @testService1");
              
              # Deactivate the same service using the echo type. This test should succeed.
              $client->mustSucceed("disnix-client --deactivate --arguments foo=foo --arguments bar=bar --type echo @testService1");
              
              # Security test. First we try to invoke a Disnix operation by an
              # unprivileged user, which should fail. Then we try the same
              # command by a privileged user, which should succeed.
              
              $client->mustFail("su - unprivileged -c 'disnix-client --print-invalid /nix/store/invalid'");
              $client->mustSucceed("su - privileged -c 'disnix-client --print-invalid /nix/store/invalid'");
              
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
              
              my $result = $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --print-invalid /nix/store/invalid");
              
              if($result =~ /\/nix\/store\/invalid/) {
                  print "/nix/store/invalid is invalid\n";
              } else {
                  die "/nix/store/invalid should be invalid\n";
              }
              
              # Check invalid path. We query a valid path from the service
              # which should return nothing in this case.
              # This test should succeed.
              
              my $result = $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --print-invalid ${pkgs.bash}");
              
              # Query requisites test. Queries the requisites of the bash shell
              # and checks whether it is part of the closure.
              # This test should succeed.
              
              my $result = $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --query-requisites ${pkgs.bash}");
              
              if($result =~ /bash/) {
                  print "${pkgs.bash} is in the closure\n";
              } else {
                  die "${pkgs.bash} should be in the closure!\n";
              }
              
              # Realise test. First a bash derivation file is instantiated,
              # then it is realised. This test should succeed.
              
              my $result = $server->mustSucceed("nix-instantiate ${nixpkgs} -A bash");
              $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --realise $result");                     
              
              # Export test. Exports the closure of the bash shell on the server
              # and then imports it on the client. This test should succeed.
              
              my $result = $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --export --remotefile ${pkgs.bash}");
              $client->mustSucceed("nix-store --import < $result");
              
              # Import test. Creates a closure of the target2Profile on the
              # client. Then it imports the closure into the Nix store of the 
              # server. This test should succeed.
              
              $server->mustFail("nix-store --check-validity @target2Profile");
              $client->mustSucceed("nix-store --export \$(nix-store -qR @target2Profile) > /root/target2Profile.closure");
              $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --import --localfile /root/target2Profile.closure");
              $server->mustSucceed("nix-store --check-validity @target2Profile");
                      
              # Set test. Adds the testtarget2 profile as only derivation into 
              # the Disnix profile. We first set the profile, then we check
              # whether the profile is part of the closure.
              # This test should succeed.
              
              $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --set --profile default @target2Profile");
              my @defaultProfileClosure = split('\n', $server->mustSucceed("nix-store -qR /nix/var/nix/profiles/disnix/default"));
              my @closure = grep("@target2Profile", @defaultProfileClosure);
              
              if("@closure" == "") {
                  print "@target2Profile is part of the closure\n";
              } else {
                  die "@target2Profile should be part of the closure\n";
              }
              
              # Query installed test. Queries the installed services in the 
              # profile, which has been set in the previous testcase.
              # testService2 should be in there. This test should succeed.
              
              my @closure = split('\n', $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --query-installed --profile default"));
              my @service = grep(/testService2/, @closure);
              
              if("@service" == "") {
                  print "@service is installed in the default profile\n";
              } else {
                  die "@service should be installed in the default profile\n";
              }
              
              # Collect garbage test. This test should succeed.
              # Testcase disabled, as this is very expensive.
              # $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --collect-garbage");

              # Lock test. This test should succeed.
              $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --lock");
              
              # Lock test. This test should fail, since the service instance is already locked
              $client->mustFail("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --lock");
              
              # Unlock test. This test should succeed, so that we can release the lock
              $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --unlock");
              
              # Unlock test. This test should fail as the lock has already been released
              $client->mustFail("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --unlock");
              
              # Use the echo type to activate a service.
              # We use the testService1 service defined in the manifest earlier
              # This test should succeed.
              $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --activate --arguments foo=foo --arguments bar=bar --type echo @testService1");
              
              # Deactivate the same service using the echo type. This test should succeed.
              $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --deactivate --arguments foo=foo --arguments bar=bar --type echo @testService1");
              
              #### Test disnix-copy-closure
              
              # Test copy closure. Here, we first dermine the closure of
              # testService3 and then we copy the closure of testService3 from
              # the client to the server. This test should succeed.
              
              my @closure = split('\n', $client->mustSucceed("nix-store -qR $manifest"));
              my @testService3 = grep(/\-testService3/, @closure);
              
              $server->mustFail("nix-store --check-validity @testService3");
              $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-copy-closure --target server --to @testService3");
              $server->mustSucceed("nix-store --check-validity @testService3");
              
              # Test copy closure. Here, we first build a package on the server,
              # which is not on the client. Then we copy the package from the
              # server to the client and we check whether the path is valid.
              
              my $result = $server->mustSucceed("nix-build ${nixpkgs} -A writeTextFile --argstr name test --argstr text 'Hello world'");
              $client->mustFail("nix-store --check-validity $result");
              $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-copy-closure --target server --from $result");
              $client->mustSucceed("nix-store --check-validity $result");
              
              #### Test disnix-gendist-roundrobin
              
              # Run disnix-gendist-roundrobin and check whether we can use the
              # generated distribution model to build the system.
              # This test should succeed.
              
              my $result = $client->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' disnix-gendist-roundrobin -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix");
              my $result = $client->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' disnix-manifest -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d $result");
              
              #### Test disnix-visualize
              
              # Makes a visualization of the roundrobin distribution. the output
              # is produced as a Hydra report. This test should succeed.
              
              $client->mustSucceed("(disnix-visualize $result) > visualize.dot");
              $client->mustSucceed("${pkgs.graphviz}/bin/dot -Tpng visualize.dot > /tmp/xchg/visualize.png");
            '';
        };
        
        deployment = simpleTest {
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
              
              if(@lines[1] != "Service on: testtarget1") {
                  die "disnix-query output line 1 does not match what we expect!\n";
              }
              
              if(@lines[3] =~ /\-testService1/) {
                  print "Found testService1 on disnix-query output line 3\n";
              } else {
                  die "disnix-query output line 3 does not contain testService1!\n";
              }
              
              if(@lines[5] != "Service on: testtarget2") {
                  die "disnix-query output line 5 does not match what we expect!\n";
              }
              
              if(@lines[7] =~ /\-testService2/) {
                  print "Found testService2 on disnix-query output line 7\n";
              } else {
                  die "disnix-query output line 7 does not contain testService2!\n";
              }
              
              if(@lines[8] =~ /\-testService3/) {
                  print "Found testService3 on disnix-query output line 8\n";
              } else {
                  die "disnix-query output line 8 does not contain testService3!\n";
              }
              
              # Check the disnix logfiles to see whether it has indeed activated
              # the services in the distribution model. This test should
              # succeed.
              
              $testtarget1->mustSucceed("[ \"\$(journalctl --no-pager --full _SYSTEMD_UNIT=disnix.service | grep \"Activate: @lines[3]\")\" != \"\" ]");
              $testtarget2->mustSucceed("[ \"\$(journalctl --no-pager --full _SYSTEMD_UNIT=disnix.service | grep \"Activate: @lines[7]\")\" != \"\" ]");
              $testtarget2->mustSucceed("[ \"\$(journalctl --no-pager --full _SYSTEMD_UNIT=disnix.service | grep \"Activate: @lines[8]\")\" != \"\" ]");
              
              # We now perform an upgrade. In this case testService2 is replaced
              # by testService2B. This test should succeed.
              
              $coordinator->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-env -s ${manifestTests}/services-composition.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-composition.nix");
              
              # Use disnix-query to see if the right services are installed on
              # the right target platforms. This test should succeed.
              
              my @lines = split('\n', $coordinator->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-query ${manifestTests}/infrastructure.nix"));
              
              if(@lines[1] != "Service on: testtarget1") {
                  die "disnix-query output line 1 does not match what we expect!\n";
              }
              
              if(@lines[3] =~ /\-testService1/) {
                  print "Found testService1 on disnix-query output line 3\n";
              } else {
                  die "disnix-query output line 3 does not contain testService1!\n";
              }
              
              if(@lines[5] != "Service on: testtarget2") {
                  die "disnix-query output line 5 does not match what we expect!\n";
              }
              
              if(@lines[7] =~ /\-testService2B/) {
                  print "Found testService2B on disnix-query output line 7\n";
              } else {
                  die "disnix-query output line 7 does not contain testService2B!\n";
              }
              
              if(@lines[8] =~ /\-testService3/) {
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
              
              my @lines = split('\n', $coordinator->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-query ${manifestTests}/infrastructure.nix"));
              
              if(@lines[1] != "Service on: testtarget1") {
                  die "disnix-query output line 1 does not match what we expect!\n";
              }
              
              if(@lines[3] =~ /\-testService1/) {
                  print "Found testService1 on disnix-query output line 3\n";
              } else {
                  die "disnix-query output line 3 does not contain testService1!\n";
              }
              
              if(@lines[4] =~ /\-testService2/) {
                  print "Found testService1 on disnix-query output line 4\n";
              } else {
                  die "disnix-query output line 4 does not contain testService1!\n";
              }
              
              if(@lines[5] =~ /\-testService3/) {
                  print "Found testService1 on disnix-query output line 5\n";
              } else {
                  die "disnix-query output line 5 does not contain testService1!\n";
              }
              
              if(@lines[7] != "Service on: testtarget2") {
                  die "disnix-query output line 7 does not match what we expect!\n";
              }
              
              if(@lines[9] =~ /\-testService2B/) {
                  print "Found testService2B on disnix-query output line 9\n";
              } else {
                  die "disnix-query output line 9 does not contain testService1!\n";
              }
              
              if(@lines[10] =~ /\-testService3/) {
                  print "Found testService3 on disnix-query output line 10\n";
              } else {
                  die "disnix-query output line 10 does not contain testService1!\n";
              }
            '';
        };
        
        distbuild = simpleTest {
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
              
              # Deploy the complete environment and build all the services on
              # the target machines. This test should succeed.
              my $result = $coordinator->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-env --build-on-targets -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix");
              
              # Checks whether the testService1 has been actually built on the
              # targets by checking the logfiles. This test should succeed.
              
              $testtarget1->mustSucceed("[ \"\$(journalctl --no-pager --full _SYSTEMD_UNIT=disnix.service | grep \"\\-testService1.drv\")\" != \"\" ]");
              
              # Checks whether the build log of testService1 is available on the
              # coordinator machine. This test should fail, since the build
              # is performed on a target machine.
              
              my $manifest = $coordinator->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' disnix-manifest -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix");
              my @closure = split('\n', $coordinator->mustSucceed("nix-store -qR result"));
              my @testService1 = grep(/\-testService1/, @closure);
                      
              $coordinator->mustFail("nix-store -l @testService1");
            '';
        };

      };
  };
in jobs
