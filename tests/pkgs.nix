{nixpkgs, dysnomia, disnix}:

let
  pkgsTests = ./pkgs;
  machine = import ./machine.nix {
    inherit dysnomia disnix;
    enableProfilePath = true;
  };
in
with import "${nixpkgs}/nixos/lib/testing-python.nix" { system = builtins.currentSystem; };

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
      start_all()

      # Initialise ssh stuff by creating a key pair for communication
      key = subprocess.check_output(
          '${pkgs.openssh}/bin/ssh-keygen -t ecdsa -f key -N ""',
          shell=True,
      )

      testtarget1.succeed("mkdir -m 700 /root/.ssh")
      testtarget1.copy_from_host("key.pub", "/root/.ssh/authorized_keys")

      testtarget2.succeed("mkdir -m 700 /root/.ssh")
      testtarget2.copy_from_host("key.pub", "/root/.ssh/authorized_keys")

      coordinator.succeed("mkdir -m 700 /root/.ssh")
      coordinator.copy_from_host("key", "/root/.ssh/id_dsa")
      coordinator.succeed("chmod 600 /root/.ssh/id_dsa")

      # Deploy the packages and check if they have become available.
      coordinator.succeed(
          "${env} disnix-env -s ${pkgsTests}/services.nix -i ${pkgsTests}/infrastructure.nix -d ${pkgsTests}/distribution.nix"
      )

      testtarget1.succeed("/nix/var/nix/profiles/disnix/default/bin/curl --help")
      testtarget1.succeed("/nix/var/nix/profiles/disnix/default/bin/strace -h")

      # Deploy an alternative configuration and check if strace has moved
      coordinator.succeed(
          "${env} disnix-env -s ${pkgsTests}/services.nix -i ${pkgsTests}/infrastructure.nix -d ${pkgsTests}/distribution-alternative.nix"
      )

      testtarget1.succeed("/nix/var/nix/profiles/disnix/default/bin/curl --help")
      testtarget2.succeed("/nix/var/nix/profiles/disnix/default/bin/strace -h")

      # Deploy a configuration in which a service declares an inter-dependency on a package. This is not allowed.
      coordinator.fail(
          "${env} disnix-manifest -s ${pkgsTests}/services-invalid.nix -i ${pkgsTests}/infrastructure.nix -d ${pkgsTests}/distribution-invalid.nix"
      )

      # Deploy a packages model and check whether the packages have become available
      coordinator.succeed(
          "${env} disnix-env -i ${pkgsTests}/infrastructure.nix -P ${pkgsTests}/pkgs.nix"
      )

      testtarget1.succeed("/nix/var/nix/profiles/disnix/default/bin/curl --help")
      testtarget2.succeed("/nix/var/nix/profiles/disnix/default/bin/strace -h")
    '';
}
