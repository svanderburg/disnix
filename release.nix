{ nixpkgs ? /etc/nixos/nixpkgs }:

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

        buildInputs = [ pkgconfig dbus_glib libxml2 libxslt getopt nixUnstable ];
      };

    build =
      { tarball ? jobs.tarball {}
      , system ? "x86_64-linux"
      }:

      with import nixpkgs { inherit system; };

      releaseTools.nixBuild {
        name = "disnix";
        src = tarball;

        buildInputs = [ pkgconfig dbus_glib libxml2 libxslt getopt nixUnstable ];
      };
      
    tests = 
      { nixos ? /etc/nixos/nixos
      , activation_scripts ? (import ../../disnix-activation-scripts-nixos/trunk/release.nix {}).build {}
      }:
      
      let
        disnix = build { system = "x86_64-linux"; };
	manifestTests = ./tests/manifest;
	machine =
	  {config, pkgs, ...}:
	    
	  {
	    # Make the Nix store in this VM writable using AUFS.  Use Linux
            # 2.6.27 because 2.6.32 doesn't work (probably we need AUFS2).
            # This should probably be moved to qemu-vm.nix.

            boot.kernelPackages = (if pkgs ? linuxPackages then
              pkgs.linuxPackages_2_6_27 else pkgs.kernelPackages_2_6_27);
            boot.extraModulePackages = [ config.boot.kernelPackages.aufs ];
            boot.initrd.availableKernelModules = [ "aufs" ];
	      
            boot.initrd.postMountCommands =
              ''
                mkdir /mnt-store-tmpfs
                mount -t tmpfs -o "mode=755" none /mnt-store-tmpfs
                mount -t aufs -o dirs=/mnt-store-tmpfs=rw:$targetRoot/nix/store=rr none $targetRoot/nix/store
              '';

            services.dbus.enable = true;
            services.dbus.packages = [ disnix ];
	    services.sshd.enable = true;
	    
	    jobs.disnix =
              { description = "Disnix server";

                startOn = "started dbus";

                script =
                  ''
                    export PATH=/var/run/current-system/sw/bin:/var/run/current-system/sw/sbin
                    export HOME=/root
	
                    ${disnix}/bin/disnix-service --activation-modules-dir=${activation_scripts}/libexec/disnix/activation-scripts
                  '';
	       };
	      
	    environment.systemPackages = [ pkgs.stdenv disnix ];
	  };	
      in
      with import "${nixos}/lib/testing.nix" { inherit nixpkgs; system = "x86_64-linux"; services = null; };
      
      {
        install = simpleTest {
	  nodes = {
	    client = machine;
	    server = machine;
	  };	    
	  testScript = 
	    ''
	      startAll;
	      
	      #### Test disnix-manifest
	      
	      # Complete inter-dependency test
	      $client->mustSucceed("NIXPKGS_ALL=${nixpkgs}/pkgs/top-level/all-packages.nix disnix-manifest -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix");
	      
	      # Incomplete inter-dependency test
	      $client->mustFail("NIXPKGS_ALL=${nixpkgs}/pkgs/top-level/all-packages.nix disnix-manifest -s ${manifestTests}/services-incomplete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix");
	      
	      # Load balancing test
	      my $manifest = $client->mustSucceed("NIXPKGS_ALL=${nixpkgs}/pkgs/top-level/all-packages.nix disnix-manifest -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-loadbalancing.nix");
	      my @closure = split('\n', $client->mustSucceed("nix-store -qR $manifest"));
	      
	      my @target1Profile = grep(/\-testTarget1/, @closure);
	      my @target2Profile = grep(/\-testTarget2/, @closure);
	      
	      my $target1ProfileClosure	= $client->mustSucceed("nix-store -qR @target1Profile");
	      my $target2ProfileClosure = $client->mustSucceed("nix-store -qR @target2Profile");
	      
	      if($target1ProfileClosure =~ /\-testService1/) {
	          print "testService1 is distributed to testTarget1 -> OK\n";
	      } else {
	          die "testService1 should be distributed to testTarget1\n";
	      }
		 
	      if($target2ProfileClosure =~ /\-testService1/) {
	          print "testService1 is distributed to testTarget2 -> OK\n";
	      } else {
	          die "testService1 should be distributed to testTarget2\n";
	      }

	      if($target1ProfileClosure =~ /\-testService2/) {
	          print "testService2 is distributed to testTarget1 -> OK\n";
	      } else {
	          die "testService2 should be distributed to testTarget1\n";
	      }
		 
	      if($target2ProfileClosure =~ /\-testService2/) {
	          print "testService2 is distributed to testTarget2 -> OK\n";
	      } else {
	          die "testService2 should be distributed to testTarget2\n";
	      }

	      if($target1ProfileClosure =~ /\-testService3/) {
	          print "testService3 is distributed to testTarget1 -> OK\n";
	      } else {
	          die "testService3 should be distributed to testTarget1\n";
	      }
		 
	      if($target2ProfileClosure =~ /\-testService3/) {
	          die "testService3 should NOT be distributed to testTarget2\n";
	      } else {
	          print "testService3 is NOT distributed to testTarget2 -> OK\n";
	      }
	      
	      # Composition test
	      my $manifest = $client->mustSucceed("NIXPKGS_ALL=${nixpkgs}/pkgs/top-level/all-packages.nix disnix-manifest -s ${manifestTests}/services-composition.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-composition.nix");
	      my $closure = $client->mustSucceed("nix-store -qR $manifest");
	      
	      if($closure =~ /\-testService2B/) {
	          print "Found testService2B";
	      } else {
	          die "testService2B not found!";
	      }
	      	  
	      # Incomplete distribution test
	      $client->mustFail("NIXPKGS_ALL=${nixpkgs}/pkgs/top-level/all-packages.nix disnix-manifest -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-incomplete.nix");
	      
	      #### Test disnix-client / dbus-service
	      	      	      
	      my $result = $client->mustSucceed("disnix-client --print-invalid /nix/store/invalid");
	      
	      if($result =~ /\/nix\/store\/invalid/) {
	          print "/nix/store/invalid is invalid\n";
	      } else {
	          die "/nix/store/invalid should be invalid\n";
	      }
	      	      
	      $client->mustSucceed("disnix-client --lock");
	      $client->mustFail("disnix-client --lock");
	      $client->mustSucceed("disnix-client --unlock");
	      $client->mustFail("disnix-client --unlock");
	      
	      #### Test disnix-ssh-client

              # Initialise ssh stuff
	      my $key=`${pkgs.openssh}/bin/ssh-keygen -t dsa -f key -N ""`;
    
              $server->mustSucceed("mkdir /root/.ssh");
              $server->mustSucceed("chmod 700 /root/.ssh");
              $server->copyFileFromHost("key.pub", "/root/.ssh/authorized_keys");
    
              $client->mustSucceed("mkdir /root/.ssh");
              $client->mustSucceed("chmod 700 /root/.ssh");
              $client->copyFileFromHost("key", "/root/.ssh/id_dsa");
              $client->mustSucceed("chmod 600 /root/.ssh/id_dsa");
	      
	      # Run test-cases
	      
	      my $result = $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --print-invalid /nix/store/invalid");
	      
	      if($result =~ /\/nix\/store\/invalid/) {
	          print "/nix/store/invalid is invalid\n";
	      } else {
	          die "/nix/store/invalid should be invalid\n";
	      }
	      
	      $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --lock");
	      $client->mustFail("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --lock");
	      $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --unlock");
	      $client->mustFail("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --unlock");
	    '';
	};
      };
  };
in jobs
