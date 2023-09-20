{nixpkgs, dysnomia, disnix}:

let
  manifestTests = ./manifest;
  machine = import ./machine.nix { inherit dysnomia disnix; };
in
with import "${nixpkgs}/nixos/lib/testing-python.nix" { system = builtins.currentSystem; };

simpleTest {
  name = "distbuild";

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
      import subprocess

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

      testtarget1.wait_for_unit("disnix")
      testtarget2.wait_for_unit("disnix")

      # Deploy the complete environment and build all the services on
      # the target machines. This test should succeed.
      result = coordinator.succeed(
          "${env} disnix-env --build-on-targets -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix"
      )

      # Checks whether the testService1 has been actually built on the
      # targets by checking the logfiles. This test should succeed.

      testtarget1.succeed(
          '[ "\$(cat /var/log/disnix/2 | grep "\\-testService1.drv")" != "" ]'
      )

      # Checks whether the build log of testService1 is available on the
      # coordinator machine. This test should fail, since the build
      # is performed on a target machine.

      manifest = coordinator.succeed(
          "${env} disnix-manifest -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix"
      )
      closure = coordinator.succeed("nix-store -qR {}".format(manifest)).split("\n")
      testService1 = [c for c in closure if "-testService1" in c][0]

      coordinator.fail("nix-store -l {}".format(testService1))

      # Perform a build process with a build model.
      coordinator.succeed(
          "${env} disnix-delegate -B ${manifestTests}/build.nix"
      )
    '';
}
