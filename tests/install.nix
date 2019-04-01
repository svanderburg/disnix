{nixpkgs, dysnomia, disnix}:

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
      env = "NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no'";
    in
    ''
      startAll;

      # Initialise ssh stuff by creating a key pair for communication
      my $key=`${pkgs.openssh}/bin/ssh-keygen -t ecdsa -f key -N ""`;

      $server->mustSucceed("mkdir -m 700 /root/.ssh");
      $server->copyFileFromHost("key.pub", "/root/.ssh/authorized_keys");

      $client->mustSucceed("mkdir -m 700 /root/.ssh");
      $client->copyFileFromHost("key", "/root/.ssh/id_dsa");
      $client->mustSucceed("chmod 600 /root/.ssh/id_dsa");

      #### Test disnix-instantiate

      # Generates a distributed derivation file. The closure should be
      # contain store derivation files. This test should succeed.

      my $result = $client->mustSucceed("${env} disnix-instantiate -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix");
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

      $client->mustSucceed("${env} disnix-manifest -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix");

      # Wrong name test. Here we have a services model with an incorrect name
      # attribute. This test should trigger an error.

      $client->mustFail("${env} disnix-manifest -s ${manifestTests}/services-wrongname.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix");

      # Incomplete inter-dependency test. Here we have a services model
      # in which an inter-dependency is not specified in the dependsOn
      # attribute. This test should trigger an error.

      $client->mustFail("${env} disnix-manifest -s ${manifestTests}/services-incomplete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix");

      # Load balancing test. Here we distribute testService1 and
      # testService2 to machines testtarget1 and testtarget2.
      # We verify this by checking whether the services testService1
      # and testService2 are in the closure of the testtarget1 and
      # testtarget2 profiles. This test should succeed.

      my $manifest = $client->mustSucceed("${env} disnix-manifest -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-loadbalancing.nix");
      @closure = split('\n', $client->mustSucceed("nix-store -qR $manifest"));

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

      $manifest = $client->mustSucceed("${env} disnix-manifest -s ${manifestTests}/services-composition.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-composition.nix");
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

      $client->mustFail("${env} disnix-manifest -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-incomplete.nix");

      # Cyclic service dependencies. Here we have a two services mutually
      # referring to each other by using the connectTo property. This should
      # not trigger an error, because the activation ordering is disregarded.

      $client->mustSucceed("${env} disnix-manifest -s ${manifestTests}/services-cyclic.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-cyclic.nix");

      # Initialize testService1
      @closure = split('\n', $client->mustSucceed("nix-store -qR $manifest"));
      my @testService1 = grep(/\-testService1/, @closure);

      # Test an architecture model, a Nix specification that specifies the
      # service and infrastructure properties and the mappings between them in
      # one configuration.

      $client->mustSucceed("${env} disnix-manifest -A ${manifestTests}/architecture.nix");

      #### Test disnix-copy-closure

      # Test copy closure. Here, we first dermine the closure of
      # testService3 and then we copy the closure of testService3 from
      # the client to the server. This test should succeed.

      @closure = split('\n', $client->mustSucceed("nix-store -qR $manifest"));
      my @testService3 = grep(/\-testService3/, @closure);

      $server->mustFail("nix-store --check-validity @testService3");
      $client->mustSucceed("${env} disnix-copy-closure --target server --to @testService3");
      $server->mustSucceed("nix-store --check-validity @testService3");

      # Test copy closure. Here, we first build a package on the server,
      # which is not on the client. Then we copy the package from the
      # server to the client and we check whether the path is valid.

      $result = $server->mustSucceed("nix-build ${nixpkgs} -A writeTextFile --argstr name test --argstr text 'Hello world'");
      $client->mustFail("nix-store --check-validity $result");
      $client->mustSucceed("${env} disnix-copy-closure --target server --from $result");
      $client->mustSucceed("nix-store --check-validity $result");

      #### Test disnix-gendist-roundrobin

      # Run disnix-gendist-roundrobin and check whether we can use the
      # generated distribution model to build the system.
      # This test should succeed.

      $result = $client->mustSucceed("${env} disnix-gendist-roundrobin -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix");
      $result = $client->mustSucceed("${env} disnix-manifest -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d $result");

      #### Test disnix-visualize

      # Makes a visualization of the roundrobin distribution. the output
      # is produced as a Hydra report. This test should succeed.

      $client->mustSucceed("(disnix-visualize $result) > visualize.dot");
      $client->mustSucceed("${pkgs.graphviz}/bin/dot -Tpng visualize.dot > /tmp/xchg/visualize.png");
    '';
}
