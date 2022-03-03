{nixpkgs, dysnomia, disnix}:

let
  manifestTests = ./manifest;
  machine = import ./machine.nix { inherit dysnomia disnix; };
in
with import "${nixpkgs}/nixos/lib/testing-python.nix" { system = builtins.currentSystem; };

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
      import subprocess

      start_all()

      # Initialise ssh stuff by creating a key pair for communication
      key = subprocess.check_output(
          '${pkgs.openssh}/bin/ssh-keygen -t ecdsa -f key -N ""',
          shell=True,
      )

      server.succeed("mkdir -m 700 /root/.ssh")
      server.copy_from_host("key.pub", "/root/.ssh/authorized_keys")

      client.succeed("mkdir -m 700 /root/.ssh")
      client.copy_from_host("key", "/root/.ssh/id_dsa")
      client.succeed("chmod 600 /root/.ssh/id_dsa")

      #### Test disnix-instantiate

      # Generates a distributed derivation file. The closure should be
      # contain store derivation files. This test should succeed.

      result = client.succeed(
          "${env} disnix-instantiate -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix"
      )
      closure = client.succeed("nix-store -qR {}".format(result))

      if ".drv" in closure:
          print("The closure of the distributed derivation contains store derivations")
      else:
          raise Exception(
              "The closure of the distributed derivation contains no store derivations!"
          )

      #### Test disnix-manifest

      # Complete inter-dependency test. Here we have a services model in
      # which both the inter-dependency specifications and distribution
      # is correct and complete. This test should succeed.

      client.succeed(
          "${env} disnix-manifest -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix"
      )

      # Wrong name test. Here we have a services model with an incorrect name
      # attribute. This test should trigger an error.

      client.fail(
          "${env} disnix-manifest -s ${manifestTests}/services-wrongname.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix"
      )

      # Incomplete inter-dependency test. Here we have a services model
      # in which an inter-dependency is not specified in the dependsOn
      # attribute. This test should trigger an error.

      client.fail(
          "${env} disnix-manifest -s ${manifestTests}/services-incomplete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix"
      )

      # Load balancing test. Here we distribute testService1 and
      # testService2 to machines testtarget1 and testtarget2.
      # We verify this by checking whether the services testService1
      # and testService2 are in the closure of the testtarget1 and
      # testtarget2 profiles. This test should succeed.

      manifest = client.succeed(
          "${env} disnix-manifest -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-loadbalancing.nix"
      )
      closure = client.succeed("nix-store -qR {}".format(manifest)).split("\n")

      target1Profile = [c for c in closure if "-testtarget1" in c][0]
      target2Profile = [c for c in closure if "-testtarget2" in c][0]

      target1ProfileClosure = client.succeed("nix-store -qR {}".format(target1Profile))
      target2ProfileClosure = client.succeed("nix-store -qR {}".format(target2Profile))

      if "-testService1" in target1ProfileClosure:
          print("testService1 is distributed to testtarget1 -> OK")
      else:
          raise Exception("testService1 should be distributed to testtarget1")

      if "-testService1" in target2ProfileClosure:
          print("testService1 is distributed to testtarget2 -> OK")
      else:
          raise Exception("testService1 should be distributed to testtarget2")

      if "-testService2" in target1ProfileClosure:
          print("testService2 is distributed to testtarget1 -> OK")
      else:
          raise Exception("testService2 should be distributed to testtarget1")

      if "-testService2" in target2ProfileClosure:
          print("testService2 is distributed to testtarget2 -> OK")
      else:
          raise Exception("testService2 should be distributed to testtarget2")

      if "-testService3" in target1ProfileClosure:
          print("testService3 is distributed to testtarget1 -> OK")
      else:
          raise Exception("testService3 should be distributed to testtarget1")

      if "-testService3" in target2ProfileClosure:
          print("testService3 should NOT be distributed to testtarget2")
      else:
          print("testService3 is NOT distributed to testtarget2 -> OK")

      # Composition test. Here we create a custom inter-dependency by
      # coupling testService3 to testService2B (instead of testService2).
      # We verify this by checking whether this service is the closure
      # of the manifest. This test should succeed.

      manifest = client.succeed(
          "${env} disnix-manifest -s ${manifestTests}/services-composition.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-composition.nix"
      )
      closure = client.succeed("nix-store -qR {}".format(manifest))

      if "-testService2B" in closure:
          print("Found testService2B")
      else:
          raise Exception("testService2B not found!")

      # Incomplete distribution test. Here we have a complete
      # inter-dependency specification in the services model, but we
      # do not distribute the service to any target.
      # This test should trigger an error.

      client.fail(
          "${env} disnix-manifest -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-incomplete.nix"
      )

      # Cyclic service dependencies. Here we have a two services mutually
      # referring to each other by using the connectTo property. This should
      # not trigger an error, because the activation ordering is disregarded.

      client.succeed(
          "${env} disnix-manifest -s ${manifestTests}/services-cyclic.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-cyclic.nix"
      )

      # Missing service in distribution model test. We use a distribution model
      # that maps a service to a machine, but the service does not exist in the
      # services model. As a result, it should fail.

      client.fail(
          "${env} disnix-manifest -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-missing.nix"
      )

      # Test a packages model, a Nix specification that specifies the content of
      # the Nix profiles for each machine

      manifestPkgs = client.succeed(
          "${env} disnix-manifest -P ${manifestTests}/target-pkgs.nix -i ${manifestTests}/infrastructure.nix"
      )
      closure = client.succeed("nix-store -qR {}".format(manifestPkgs))

      if "curl" in closure:
          print("Found curl package in closure!")
      else:
          raise Exception("curl should be in the closure!")

      # Test an architecture model, a Nix specification that specifies the
      # service and infrastructure properties and the mappings between them in
      # one configuration.

      client.succeed(
          "${env} disnix-manifest -A ${manifestTests}/architecture.nix"
      )

      # Test a parametrized services model. The name of the resulting service
      # (myService) should be different than the default (testService).

      manifest = client.succeed(
          "${env} disnix-manifest -s ${manifestTests}/services-parametrized.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-parametrized.nix --extra-params '{ prefix = \"myService\"; }'"
      )
      closure = client.succeed("nix-store -qR {}".format(manifest))

      if "-myService" in closure:
          print("Found myService package in the closure!")
      else:
          raise Exception("myService should be in the closure: {}".format(closure))

      #### Test disnix-copy-closure

      manifest = client.succeed(
          "${env} disnix-manifest -s ${manifestTests}/services-composition.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-composition.nix"
      )

      # Test copy closure. Here, we first dermine the closure of
      # testService3 and then we copy the closure of testService3 from
      # the client to the server. This test should succeed.

      closure = client.succeed("nix-store -qR {}".format(manifest)).split("\n")
      testService3 = [c for c in closure if "-testService3" in c][0]

      server.fail("nix-store --check-validity {}".format(testService3))
      client.succeed(
          "${env} disnix-copy-closure --target server --to {}".format(
              testService3
          )
      )
      server.succeed("nix-store --check-validity {}".format(testService3))

      # Test copy closure. Here, we first build a package on the server,
      # which is not on the client. Then we copy the package from the
      # server to the client and we check whether the path is valid.

      result = server.succeed(
          "nix-build ${nixpkgs} -A writeTextFile --argstr name test --argstr text 'Hello world'"
      )
      client.fail("nix-store --check-validity {}".format(result))
      client.succeed(
          "${env} disnix-copy-closure --target server --from {}".format(
              result
          )
      )
      client.succeed("nix-store --check-validity {}".format(result))

      #### Test disnix-gendist-roundrobin

      # Run disnix-gendist-roundrobin and check whether we can use the
      # generated distribution model to build the system.
      # This test should succeed.

      result = client.succeed(
          "${env} disnix-gendist-roundrobin -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix"
      )
      result = client.succeed(
          "${env} disnix-manifest -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d {}".format(
              result
          )
      )

      #### Test disnix-visualize

      # Makes a visualization of the roundrobin distribution. the output
      # is produced as a Hydra report. This test should succeed.

      client.succeed("(disnix-visualize {}) > visualize.dot".format(result))
      client.succeed(
          "${pkgs.graphviz}/bin/dot -Tpng visualize.dot > /tmp/xchg/visualize.png"
      )
    '';
}
