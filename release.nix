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
      , system ? "x86_64-linux"
      }:
      
      let
        disnix = build { inherit system; };
	manifestTests = ./tests/manifest;
      in
      with import "${nixos}/lib/testing.nix" { inherit nixpkgs system; services = null; };
      
      {
        install = simpleTest {
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

	      environment.systemPackages = [ pkgs.stdenv disnix ];
	    }
	    ;
	  testScript = 
	    ''
	      # Complete inter-dependency test
	      $machine->mustSucceed("NIXPKGS_ALL=${nixpkgs}/pkgs/top-level/all-packages.nix disnix-manifest -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix");
	      
	      # Incomplete inter-dependency test
	      $machine->mustFail("NIXPKGS_ALL=${nixpkgs}/pkgs/top-level/all-packages.nix disnix-manifest -s ${manifestTests}/services-incomplete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix");
	      
	      # Load balancing test
	      my $manifest = $machine->mustSucceed("NIXPKGS_ALL=${nixpkgs}/pkgs/top-level/all-packages.nix disnix-manifest -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-loadbalancing.nix");
	      my @closure = split('\n', $machine->mustSucceed("nix-store -qR $manifest"));
	      
	      my @target1Profile = grep(/\-testTarget1/, @closure);
	      my @target2Profile = grep(/\-testTarget2/, @closure);
	      
	      my $target1ProfileClosure	= $machine->mustSucceed("nix-store -qR @target1Profile");
	      my $target2ProfileClosure = $machine->mustSucceed("nix-store -qR @target2Profile");
	      
	      if($target1ProfileClosure =~ /\-testService1/) {
	          print "testService1 is distributed to testTarget1 -> OK";
	      } else {
	          die "testService1 should be distributed to testTarget1";
	      }
		 
	      if($target2ProfileClosure =~ /\-testService1/) {
	          print "testService1 is distributed to testTarget2 -> OK";
	      } else {
	          die "testService1 should be distributed to testTarget2";
	      }

	      if($target1ProfileClosure =~ /\-testService2/) {
	          print "testService2 is distributed to testTarget1 -> OK";
	      } else {
	          die "testService2 should be distributed to testTarget1";
	      }
		 
	      if($target2ProfileClosure =~ /\-testService2/) {
	          print "testService2 is distributed to testTarget2 -> OK";
	      } else {
	          die "testService2 should be distributed to testTarget2";
	      }

	      if($target1ProfileClosure =~ /\-testService3/) {
	          print "testService3 is distributed to testTarget1 -> OK";
	      } else {
	          die "testService3 should be distributed to testTarget1";
	      }
		 
	      if($target2ProfileClosure =~ /\-testService3/) {
	          die "testService3 should NOT be distributed to testTarget2";
	      } else {
	          print "testService3 is NOT distributed to testTarget2 -> OK";	          
	      }
	      
	      # Composition test
	      my $manifest = $machine->mustSucceed("NIXPKGS_ALL=${nixpkgs}/pkgs/top-level/all-packages.nix disnix-manifest -s ${manifestTests}/services-composition.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-composition.nix");
	      my $closure = $machine->mustSucceed("nix-store -qR $manifest");
	      
	      if($closure =~ /\-testService2B/) {
	          print "Found testService2B";
	      } else {
	          die "testService2B not found!";
	      }
	      	  
	      # Incomplete distribution test
	      $machine->mustFail("NIXPKGS_ALL=${nixpkgs}/pkgs/top-level/all-packages.nix disnix-manifest -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-incomplete.nix");
	    '';
	};
      };
  };
in jobs
