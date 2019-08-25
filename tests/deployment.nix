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
    let
      env = "NIX_PATH='nixpkgs=${nixpkgs}' SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no'";
    in
    ''
      startAll;

      # Initialise ssh stuff by creating a key pair for communication
      my $key=`${pkgs.openssh}/bin/ssh-keygen -t ecdsa -f key -N ""`;

      $testtarget1->mustSucceed("mkdir -m 700 /root/.ssh");
      $testtarget1->copyFileFromHost("key.pub", "/root/.ssh/authorized_keys");

      $testtarget2->mustSucceed("mkdir -m 700 /root/.ssh");
      $testtarget2->copyFileFromHost("key.pub", "/root/.ssh/authorized_keys");

      $coordinator->mustSucceed("mkdir -m 700 /root/.ssh");
      $coordinator->copyFileFromHost("key", "/root/.ssh/id_dsa");
      $coordinator->mustSucceed("chmod 600 /root/.ssh/id_dsa");

      # Do a rollback. Since there is nothing deployed, it should fail.
      $coordinator->mustFail("${env} disnix-env --rollback");

      # Use disnix-env to perform a new installation that fails.
      # It should properly do a rollback.
      $coordinator->mustFail("${env} disnix-env -s ${manifestTests}/services-fail.nix -i ${manifestTests}/infrastructure-single.nix -d ${manifestTests}/distribution-fail.nix");

      # Use disnix-env to perform a new installation.
      # This test should succeed.
      $coordinator->mustSucceed("${env} disnix-env -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix");

      # Check if we have one profile generation.
      my $result = $coordinator->mustSucceed("${env} disnix-env --list-generations | wc -l");

      if($result == 1) {
          print("We have 1 profile generation\n");
      } else {
          die "We should have 1 profile generation, instead we have: $result";
      }

      # Do another rollback. Since there is no previous deployment, it should fail.
      $coordinator->mustFail("${env} disnix-env --rollback");

      # Use disnix-query to see if the right services are installed on
      # the right target platforms. This test should succeed.

      $coordinator->mustSucceed("${env} disnix-query -f xml ${manifestTests}/infrastructure.nix > query.xml");

      my $testService1PkgElem = $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget1']/profileManifest/services/service[name='testService1']/pkg\" query.xml");
      my $testService2PkgElem = $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget2']/profileManifest/services/service[name='testService2']/pkg\" query.xml");
      my $testService3PkgElem = $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget2']/profileManifest/services/service[name='testService3']/pkg\" query.xml");

      # Check the disnix logfiles to see whether it has indeed activated
      # the services in the distribution model. This test should
      # succeed.

      $testtarget1->mustSucceed("[ \"\$(cat /var/log/disnix/8 | grep \"activate: ".(substr $testService1PkgElem, 5, -7)."\")\" != \"\" ]");
      $testtarget2->mustSucceed("[ \"\$(cat /var/log/disnix/3 | grep \"activate: ".(substr $testService2PkgElem, 5, -7)."\")\" != \"\" ]");
      $testtarget2->mustSucceed("[ \"\$(cat /var/log/disnix/4 | grep \"activate: ".(substr $testService3PkgElem, 5, -7)."\")\" != \"\" ]");

      # Check if there is only one generation link in the coordinator profile
      # folder and one generation link in the target profiles folder on each
      # machine.

      $result = $coordinator->mustSucceed("ls /nix/var/nix/profiles/per-user/root/disnix-coordinator | wc -l");

      if($result == 2) {
          print "We have only one generation symlink on the coordinator!\n";
      } else {
          die "We should have one generation symlink on the coordinator!";
      }

      $result = $testtarget1->mustSucceed("ls /nix/var/nix/profiles/disnix | wc -l");

      if($result == 2) {
          print "We have only one generation symlink on target1!\n";
      } else {
          die "We should have one generation symlink on target1!";
      }

      $result = $testtarget2->mustSucceed("ls /nix/var/nix/profiles/disnix | wc -l");

      if($result == 2) {
          print "We have only one generation symlink on target2!\n";
      } else {
          die "We should have one generation symlink on target2!";
      }

      # We repeat the previous disnix-env command. No changes should be
      # performed. Moreover, we should still have one coordinator profile
      # and one target profile per machine. This test should succeed.

      $coordinator->mustSucceed("${env} disnix-env -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix");

      $result = $coordinator->mustSucceed("${env} disnix-env --list-generations | wc -l");

      if($result == 1) {
          print "We have one generation symlink\n";
      } else {
          die "We should have one generation symlink, instead we have: $result"
      }

      $result = $coordinator->mustSucceed("ls /nix/var/nix/profiles/per-user/root/disnix-coordinator | wc -l");

      if($result == 2) {
          print "We have only one generation symlink on the coordinator!\n";
      } else {
          die "We should have one generation symlink on the coordinator!";
      }

      $result = $testtarget1->mustSucceed("ls /nix/var/nix/profiles/disnix | wc -l");

      if($result == 2) {
          print "We have only one generation symlink on target1!\n";
      } else {
          die "We should have one generation symlink on target1!";
      }

      $result = $testtarget2->mustSucceed("ls /nix/var/nix/profiles/disnix | wc -l");

      if($result == 2) {
          print "We have only one generation symlink on target2!\n";
      } else {
          die "We should have one generation symlink on target2!";
      }

      # We now perform an upgrade by moving testService2 to another machine.
      # This test should succeed.
      $coordinator->mustSucceed("${env} disnix-env -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-reverse.nix");

      $result = $coordinator->mustSucceed("${env} disnix-env --list-generations | wc -l");

      if($result == 2) {
          print "We have two generation symlinks\n";
      } else {
          die "We should have two generation symlinks, instead we have: $result"
      }

      $coordinator->mustSucceed("${env} disnix-query -f xml ${manifestTests}/infrastructure.nix > query.xml");

      $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget1']/profileManifest/services/service[name='testService1']/name\" query.xml");
      $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget1']/profileManifest/services/service[name='testService2']/name\" query.xml");
      $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget2']/profileManifest/services/service[name='testService3']/name\" query.xml");

      # Since the deployment state has changes, we should now have a new
      # profile entry added on the coordinator and target machines.
      $result = $coordinator->mustSucceed("ls /nix/var/nix/profiles/per-user/root/disnix-coordinator | wc -l");

      if($result == 3) {
          print "We have two generation symlinks on the coordinator!\n";
      } else {
          die "We should have two generation symlinks on the coordinator!";
      }

      $result = $testtarget1->mustSucceed("ls /nix/var/nix/profiles/disnix | wc -l");

      if($result == 3) {
          print "We have two generation symlinks on target1!\n";
      } else {
          die "We should have two generation symlinks on target1!";
      }

      $result = $testtarget2->mustSucceed("ls /nix/var/nix/profiles/disnix | wc -l");

      if($result == 3) {
          print "We have two generation symlinks on target2!\n";
      } else {
          die "We should have two generation symlinks on target2!";
      }

      # Now we undo the upgrade again by moving testService2 back.
      # This test should succeed.
      $coordinator->mustSucceed("${env} disnix-env -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix");

      $coordinator->mustSucceed("${env} disnix-query -f xml ${manifestTests}/infrastructure.nix > query.xml");

      $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget1']/profileManifest/services/service[name='testService1']/name\" query.xml");
      $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget2']/profileManifest/services/service[name='testService2']/name\" query.xml");
      $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget2']/profileManifest/services/service[name='testService3']/name\" query.xml");

      # We now perform an upgrade. In this case testService2 is replaced
      # by testService2B. This test should succeed.

      $coordinator->mustSucceed("${env} disnix-env -s ${manifestTests}/services-composition.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-composition.nix --show-trace");

      # Use disnix-query to see if the right services are installed on
      # the right target platforms. This test should succeed.

      $coordinator->mustSucceed("${env} disnix-query -f xml ${manifestTests}/infrastructure.nix > query.xml");

      $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget1']/profileManifest/services/service[name='testService1']/name\" query.xml");
      $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget2']/profileManifest/services/service[name='testService2B']/name\" query.xml");
      $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget2']/profileManifest/services/service[name='testService3']/name\" query.xml");

      # We now perform another upgrade. We move all services from
      # testTarget2 to testTarget1. In this case testTarget2 has become
      # unavailable (not defined in infrastructure model), so the service
      # should not be deactivated on testTarget2. This test should
      # succeed.

      $coordinator->mustSucceed("${env} disnix-env -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure-single.nix -d ${manifestTests}/distribution-single.nix > result");
      $coordinator->mustSucceed("[ \"\$(grep \"Skip service\" result | grep \"testService2B\" | grep \"testtarget2\")\" != \"\" ]");
      $coordinator->mustSucceed("[ \"\$(grep \"Skip service\" result | grep \"testService3\" | grep \"testtarget2\")\" != \"\" ]");

      # Use disnix-query to check whether testService{1,2,3} are
      # available on testtarget1 and testService{2B,3} are still
      # deployed on testtarget2. This test should succeed.

      $coordinator->mustSucceed("${env} disnix-query -f xml ${manifestTests}/infrastructure.nix > query.xml");

      $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget1']/profileManifest/services/service[name='testService1']/name\" query.xml");
      $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget1']/profileManifest/services/service[name='testService2']/name\" query.xml");
      $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget1']/profileManifest/services/service[name='testService3']/name\" query.xml");

      # Do an upgrade with a transitive dependency. In this test we change the
      # binding of testService3 to a testService2 instance that changes its
      # interdependency from testService1 to testService1B. As a result, both
      # testService2 and testService3 must be redeployed.
      # This test should succeed.

      $coordinator->mustSucceed("${env} disnix-env -s ${manifestTests}/services-transitivecomposition.nix -i ${manifestTests}/infrastructure-single.nix -d ${manifestTests}/distribution-transitivecomposition.nix > result");

      # Use disnix-query to check whether testService{1,1B,2,3} are
      # available on testtarget1. This test should succeed.

      $coordinator->mustSucceed("${env} disnix-query -f xml ${manifestTests}/infrastructure.nix > query.xml");

      $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget1']/profileManifest/services/service[name='testService1']/name\" query.xml");
      $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget1']/profileManifest/services/service[name='testService1B']/name\" query.xml");
      $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget1']/profileManifest/services/service[name='testService2']/name\" query.xml");
      $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget1']/profileManifest/services/service[name='testService3']/name\" query.xml");

      # Do an upgrade to an environment containing only one service that's a running process.
      # This test should succeed.
      $coordinator->mustSucceed("${env} disnix-env -s ${manifestTests}/services-echo.nix -i ${manifestTests}/infrastructure-single.nix -d ${manifestTests}/distribution-process.nix > result");

      # Do a type upgrade. We change the type of the process from 'echo' to
      # 'wrapper', triggering a redeployment. This test should succeed.
      $coordinator->mustSucceed("${env} disnix-env -s ${manifestTests}/services-process.nix -i ${manifestTests}/infrastructure-single.nix -d ${manifestTests}/distribution-process.nix > result");

      # Check if the 'process' has written the tmp file.
      # This test should succeed.
      $testtarget1->mustSucceed("sleep 10 && [ -f /tmp/process_out ] && rm /tmp/process_out");

      # Do an upgrade that intentionally fails.
      # This test should fail.
      $coordinator->mustFail("${env} disnix-env -s ${manifestTests}/services-fail.nix -i ${manifestTests}/infrastructure-single.nix -d ${manifestTests}/distribution-fail.nix > result");

      # Check if the 'process' has written the tmp file again.
      # This test should succeed.
      $testtarget1->mustSucceed("sleep 10 && [ -f /tmp/process_out ] && rm /tmp/process_out");

      # Roll back to the previously deployed configuration
      $coordinator->mustSucceed("${env} disnix-env --rollback");

      # We should have one service of type echo now on the testtarget1 machine
      $testtarget1->mustSucceed("[ \"\$(xmllint --format /nix/var/nix/profiles/disnix/default/profilemanifest.xml | grep \"echo\" | wc -l)\" = \"2\" ]");

      # Roll back to the first deployed configuration
      $coordinator->mustSucceed("${env} disnix-env --switch-to-generation 1");

      # Use disnix-query to see if the right services are installed on
      # the right target platforms. This test should succeed.

      $coordinator->mustSucceed("${env} disnix-query -f xml ${manifestTests}/infrastructure.nix > query.xml");

      $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget1']/profileManifest/services/service[name='testService1']/name\" query.xml");
      $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget2']/profileManifest/services/service[name='testService2']/name\" query.xml");
      $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget2']/profileManifest/services/service[name='testService3']/name\" query.xml");

      # Test the alternative and more verbose distribution, which does
      # the same thing as the simple distribution.
      $coordinator->mustSucceed("${env} disnix-env -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-alternate.nix");

      # Test multi container deployment.
      $coordinator->mustSucceed("${env} disnix-env -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-multicontainer.nix");

      # Use disnix-query to see if the right services are installed on
      # the right target platforms. This test should succeed.

      $coordinator->mustSucceed("${env} disnix-query -f xml ${manifestTests}/infrastructure.nix > query.xml");

      $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget1']/profileManifest/services/service[name='testService1']/name\" query.xml");
      $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget1']/profileManifest/services/service[name='testService3']/name\" query.xml");
      $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget2']/profileManifest/services/service[name='testService2']/name\" query.xml");
      $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget2']/profileManifest/services/service[name='testService3']/name\" query.xml");

      # Remove old generation test. We remove one profile generation and we
      # check if it has been successfully removed

      my $oldNumOfGenerations = $coordinator->mustSucceed("${env} disnix-env --list-generations | wc -l");
      $coordinator->mustSucceed("${env} disnix-env --delete-generations 1");
      my $newNumOfGenerations = $coordinator->mustSucceed("${env} disnix-env --list-generations | wc -l");

      if($newNumOfGenerations == $oldNumOfGenerations - 1) {
          print "We have successfully removed one generation!\n";
      } else {
          die "We seem to have removed more than one generation, new: $newNumOfGenerations, old: $oldNumOfGenerations";
      }

      # Test disnix-capture-infra. Capture the container properties of all
      # machines and generate an infrastructure expression from it. It should
      # contain: "foo" = "bar"; twice.
      $result = $coordinator->mustSucceed("${env} disnix-capture-infra ${manifestTests}/infrastructure.nix | grep '\"foo\" = \"bar\"' | wc -l");

      if($result == 2) {
         print "We have foo=bar twice in the infrastructure model!\n";
      } else {
          die "We should have foo=bar twice in the infrastructure model. Instead, we have: $result";
      }

      # It should also provide a list of supported types twice.
      $result = $coordinator->mustSucceed("${env} disnix-capture-infra ${manifestTests}/infrastructure.nix | grep '\"supportedTypes\" = \\[ \"process\"' | wc -l");

      if($result == 2) {
         print "We have supportedTypes twice in the infrastructure model!\n";
      } else {
          die "We should have supportedTypes twice in the infrastructure model. Instead, we have: $result";
      }

      # Test disnix-reconstruct. Because nothing has changed the coordinator
      # profile should remain identical.

      $oldNumOfGenerations = $coordinator->mustSucceed("ls /nix/var/nix/profiles/per-user/root/disnix-coordinator | wc -l");
      $coordinator->mustSucceed("${env} disnix-reconstruct ${manifestTests}/infrastructure.nix");
      $newNumOfGenerations = $coordinator->mustSucceed("ls /nix/var/nix/profiles/per-user/root/disnix-coordinator | wc -l");

      if($oldNumOfGenerations == $newNumOfGenerations) {
          print "The amount of manifest generations remained the same!\n";
      } else {
          die "The amount of manifest generations should remain the same!";
      }

      # Test disnix-reconstruct. First, we remove the old manifests. They
      # should have been reconstructed.

      $result = $coordinator->mustSucceed("${env} disnix-env --delete-all-generations | wc -l");

      if($result == 0) {
          print "We have no symlinks\n";
      } else {
          die "We should have no symlinks, instead we have: $result";
      }

      $coordinator->mustSucceed("${env} disnix-reconstruct ${manifestTests}/infrastructure.nix");
      $result = $coordinator->mustSucceed("ls /nix/var/nix/profiles/per-user/root/disnix-coordinator | wc -l");

      if($result == 2) {
          print "We have a reconstructed manifest!\n";
      } else {
          die "We don't have any reconstructed manifests!";
      }

      # Use disnix-env to perform another installation combined with
      # supplemental packages.
      $coordinator->mustSucceed("${env} disnix-env -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix -P ${manifestTests}/pkgs.nix");

      # Use disnix-query to see if the right services are installed on
      # the right target platforms. This test should succeed.

      $coordinator->mustSucceed("${env} disnix-query -f xml ${manifestTests}/infrastructure.nix > query.xml");

      $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget1']/profileManifest/services/service[name='testService1']/name\" query.xml");
      $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget2']/profileManifest/services/service[name='testService2']/name\" query.xml");
      $coordinator->mustSucceed("xmllint --xpath \"/profileManifestTargets/target[\@name='testtarget2']/profileManifest/services/service[name='testService3']/name\" query.xml");

      # Check if the packages have been properly installed as well
      $testtarget1->mustSucceed("/nix/var/nix/profiles/disnix/default/bin/curl --help");
      $testtarget2->mustSucceed("/nix/var/nix/profiles/disnix/default/bin/strace -h");

      # Use disnix-diagnose to execute a remote command
      # Check if this_component environment variable refers to testService1
      $coordinator->mustSucceed("${env} disnix-diagnose -S testService1 --command 'echo \$this_component' | grep testService1");

      # Use disnix-diagnose to execute a remote command on a specific target
      # Check if this_component environment variable refers to testService1
      $coordinator->mustSucceed("${env} disnix-diagnose -S testService1 -t testtarget1 --command 'echo \$this_component' | grep testService1");

      # Use disnix-diagnose to execute a remote command on a specific target and container
      # Check if this_component environment variable refers to testService1
      $coordinator->mustSucceed("${env} disnix-diagnose -S testService1 -t testtarget1 -c echo --command 'echo \$this_component' | grep testService1");

      # Use disnix-diagnose to execute a remote command on the wrong target. This should fail
      $coordinator->mustFail("${env} disnix-diagnose -S testService1 -t testtarget2 --command 'echo \$this_component' | grep testService1");

      # Deploy using a deployment model that only deploys profiles. It should have the right packages installed.
      $coordinator->mustSucceed("${env} disnix-env -D ${manifestTests}/deployment.nix");

      $testtarget1->mustSucceed("/nix/var/nix/profiles/disnix/default/bin/curl --help");
      $testtarget2->mustSucceed("/nix/var/nix/profiles/disnix/default/bin/strace -h");
    '';
}
