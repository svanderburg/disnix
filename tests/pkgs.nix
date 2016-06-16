{nixpkgs, dysnomia, disnix}:

let
  pkgsTests = ./pkgs;
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
      let
        env = "NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no'";
      in
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
        
        # Deploy the packages and check if they have become available.
        $coordinator->mustSucceed("${env} disnix-env -s ${pkgsTests}/services.nix -i ${pkgsTests}/infrastructure.nix -d ${pkgsTests}/distribution.nix");
        
        $testtarget1->mustSucceed("/nix/var/nix/profiles/disnix/default/bin/curl --help");
        $testtarget1->mustSucceed("/nix/var/nix/profiles/disnix/default/bin/strace -h");
        
        # Deploy an alternative configuration and check if strace has moved
        $coordinator->mustSucceed("${env} disnix-env -s ${pkgsTests}/services.nix -i ${pkgsTests}/infrastructure.nix -d ${pkgsTests}/distribution-alternative.nix");
        
        $testtarget1->mustSucceed("/nix/var/nix/profiles/disnix/default/bin/curl --help");
        $testtarget2->mustSucceed("/nix/var/nix/profiles/disnix/default/bin/strace -h");
        
        # Deploy a configuration in which a service declares an inter-dependency. This is not allowed.
        $coordinator->mustFail("${env} disnix-manifest -s ${pkgsTests}/services-invalid.nix -i ${pkgsTests}/infrastructure.nix -d ${pkgsTests}/distribution-invalid.nix");
      '';
  }
