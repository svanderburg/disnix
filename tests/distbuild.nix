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
        
        # Deploy the complete environment and build all the services on
        # the target machines. This test should succeed.
        my $result = $coordinator->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-env --build-on-targets -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix");
        
        # Checks whether the testService1 has been actually built on the
        # targets by checking the logfiles. This test should succeed.
        
        $testtarget1->mustSucceed("[ \"\$(cat /var/log/disnix/1 | grep \"\\-testService1.drv\")\" != \"\" ]");
        
        # Checks whether the build log of testService1 is available on the
        # coordinator machine. This test should fail, since the build
        # is performed on a target machine.
        
        my $manifest = $coordinator->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' disnix-manifest -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix");
        my @closure = split('\n', $coordinator->mustSucceed("nix-store -qR result"));
        my @testService1 = grep(/\-testService1/, @closure);
        
        $coordinator->mustFail("nix-store -l @testService1");
    '';
  }
