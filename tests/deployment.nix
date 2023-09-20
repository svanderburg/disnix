{nixpkgs, dysnomia, disnix}:

let
  manifestTests = ./manifest;
  machine = import ./machine.nix { inherit dysnomia disnix; };
in
with import "${nixpkgs}/nixos/lib/testing-python.nix" { system = builtins.currentSystem; };

simpleTest {
  name = "deployment";

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

      # Do a rollback. Since there is nothing deployed, it should fail.
      coordinator.fail(
          "${env} disnix-env --rollback"
      )

      # Use disnix-env to perform a new installation that fails.
      # It should properly do a rollback.
      coordinator.fail(
          "${env} disnix-env -s ${manifestTests}/services-fail.nix -i ${manifestTests}/infrastructure-single.nix -d ${manifestTests}/distribution-fail.nix"
      )

      # Use disnix-env to perform a new installation.
      # This test should succeed.
      coordinator.succeed(
          "${env} disnix-env -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix"
      )

      # Check if we have one profile generation.
      result = coordinator.succeed(
          "${env} disnix-env --list-generations | wc -l"
      )

      if int(result) == 1:
          print("We have 1 profile generation")
      else:
          raise Exception(
              "We should have 1 profile generation, instead we have: {}".format(result)
          )

      # Do another rollback. Since there is no previous deployment, it should fail.
      coordinator.fail(
          "${env} disnix-env --rollback"
      )

      # Use disnix-query to see if the right services are installed on
      # the right target platforms. This test should succeed.

      coordinator.succeed(
          "${env} disnix-query -f xml ${manifestTests}/infrastructure.nix > query.xml"
      )

      testService1PkgElem = coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget1']/profileManifest/services/service[name='testService1']/pkg\" query.xml"
      )
      testService2PkgElem = coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget2']/profileManifest/services/service[name='testService2']/pkg\" query.xml"
      )
      testService3PkgElem = coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget2']/profileManifest/services/service[name='testService3']/pkg\" query.xml"
      )

      # Check the disnix logfiles to see whether it has indeed activated
      # the services in the distribution model. This test should
      # succeed.

      testtarget1.succeed(
          '[ "$(cat /var/log/disnix/8 | grep "activate: {}")" != "" ]'.format(
              testService1PkgElem[5:-7]
          )
      )
      testtarget2.succeed(
          '[ "$(cat /var/log/disnix/3 | grep "activate: {}")" != "" ]'.format(
              testService2PkgElem[5:-7]
          )
      )
      testtarget2.succeed(
          '[ "$(cat /var/log/disnix/4 | grep "activate: {}")" != "" ]'.format(
              testService3PkgElem[5:-7]
          )
      )

      # Check if there is only one generation link in the coordinator profile
      # folder and one generation link in the target profiles folder on each
      # machine.

      result = coordinator.succeed(
          "ls /nix/var/nix/profiles/per-user/root/disnix-coordinator | wc -l"
      )

      if int(result) == 2:
          print("We have only one generation symlink on the coordinator!")
      else:
          raise Exception("We should have one generation symlink on the coordinator!")

      result = testtarget1.succeed("ls /nix/var/nix/profiles/disnix | wc -l")

      if int(result) == 2:
          print("We have only one generation symlink on target1!")
      else:
          raise Exception("We should have one generation symlink on target1!")

      result = testtarget2.succeed("ls /nix/var/nix/profiles/disnix | wc -l")

      if int(result) == 2:
          print("We have only one generation symlink on target2!")
      else:
          raise Exception("We should have one generation symlink on target2!")

      # We repeat the previous disnix-env command. No changes should be
      # performed. Moreover, we should still have one coordinator profile
      # and one target profile per machine. This test should succeed.

      coordinator.succeed(
          "${env} disnix-env -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix"
      )

      result = coordinator.succeed(
          "${env} disnix-env --list-generations | wc -l"
      )

      if int(result) == 1:
          print("We have one generation symlink")
      else:
          raise Exception(
              "We should have one generation symlink, instead we have: {}".format(result)
          )

      result = coordinator.succeed(
          "ls /nix/var/nix/profiles/per-user/root/disnix-coordinator | wc -l"
      )

      if int(result) == 2:
          print("We have only one generation symlink on the coordinator!")
      else:
          raise Exception("We should have one generation symlink on the coordinator!")

      result = testtarget1.succeed("ls /nix/var/nix/profiles/disnix | wc -l")

      if int(result) == 2:
          print("We have only one generation symlink on target1!")
      else:
          raise Exception("We should have one generation symlink on target1!")

      result = testtarget2.succeed("ls /nix/var/nix/profiles/disnix | wc -l")

      if int(result) == 2:
          print("We have only one generation symlink on target2!")
      else:
          raise Exception("We should have one generation symlink on target2!")

      # We now perform an upgrade by moving testService2 to another machine.
      # This test should succeed.
      coordinator.succeed(
          "${env} disnix-env -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-reverse.nix"
      )

      result = coordinator.succeed(
          "${env} disnix-env --list-generations | wc -l"
      )

      if int(result) == 2:
          print("We have two generation symlinks")
      else:
          raise Exception(
              "We should have two generation symlinks, instead we have: {}".format(result)
          )

      coordinator.succeed(
          "${env} disnix-query -f xml ${manifestTests}/infrastructure.nix > query.xml"
      )

      coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget1']/profileManifest/services/service[name='testService1']/name\" query.xml"
      )
      coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget1']/profileManifest/services/service[name='testService2']/name\" query.xml"
      )
      coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget2']/profileManifest/services/service[name='testService3']/name\" query.xml"
      )

      # Since the deployment state has changes, we should now have a new
      # profile entry added on the coordinator and target machines.
      result = coordinator.succeed(
          "ls /nix/var/nix/profiles/per-user/root/disnix-coordinator | wc -l"
      )

      if int(result) == 3:
          print("We have two generation symlinks on the coordinator!")
      else:
          raise Exception("We should have two generation symlinks on the coordinator!")

      result = testtarget1.succeed("ls /nix/var/nix/profiles/disnix | wc -l")

      if int(result) == 3:
          print("We have two generation symlinks on target1!")
      else:
          raise Exception("We should have two generation symlinks on target1!")

      result = testtarget2.succeed("ls /nix/var/nix/profiles/disnix | wc -l")

      if int(result) == 3:
          print("We have two generation symlinks on target2!")
      else:
          raise Exception("We should have two generation symlinks on target2!")

      # Now we undo the upgrade again by moving testService2 back.
      # This test should succeed.
      coordinator.succeed(
          "${env} disnix-env -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix"
      )

      coordinator.succeed(
          "${env} disnix-query -f xml ${manifestTests}/infrastructure.nix > query.xml"
      )

      coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget1']/profileManifest/services/service[name='testService1']/name\" query.xml"
      )
      coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget2']/profileManifest/services/service[name='testService2']/name\" query.xml"
      )
      coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget2']/profileManifest/services/service[name='testService3']/name\" query.xml"
      )

      # We now perform an upgrade. In this case testService2 is replaced
      # by testService2B. This test should succeed.

      coordinator.succeed(
          "${env} disnix-env -s ${manifestTests}/services-composition.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-composition.nix"
      )

      # Use disnix-query to see if the right services are installed on
      # the right target platforms. This test should succeed.

      coordinator.succeed(
          "${env} disnix-query -f xml ${manifestTests}/infrastructure.nix > query.xml"
      )

      coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget1']/profileManifest/services/service[name='testService1']/name\" query.xml"
      )
      coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget2']/profileManifest/services/service[name='testService2B']/name\" query.xml"
      )
      coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget2']/profileManifest/services/service[name='testService3']/name\" query.xml"
      )

      # We now perform another upgrade. We move all services from
      # testTarget2 to testTarget1. In this case testTarget2 has become
      # unavailable (not defined in infrastructure model), so the service
      # should not be deactivated on testTarget2. This test should
      # succeed.

      coordinator.succeed(
          "${env} disnix-env -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure-single.nix -d ${manifestTests}/distribution-single.nix > result"
      )
      coordinator.succeed(
          '[ "$(grep "Skip service" result | grep "testService2B" | grep "testtarget2")" != "" ]'
      )
      coordinator.succeed(
          '[ "$(grep "Skip service" result | grep "testService3" | grep "testtarget2")" != "" ]'
      )

      # Use disnix-query to check whether testService{1,2,3} are
      # available on testtarget1 and testService{2B,3} are still
      # deployed on testtarget2. This test should succeed.

      coordinator.succeed(
          "${env} disnix-query -f xml ${manifestTests}/infrastructure.nix > query.xml"
      )

      coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget1']/profileManifest/services/service[name='testService1']/name\" query.xml"
      )
      coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget1']/profileManifest/services/service[name='testService2']/name\" query.xml"
      )
      coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget1']/profileManifest/services/service[name='testService3']/name\" query.xml"
      )

      # Do an upgrade with a transitive dependency. In this test we change the
      # binding of testService3 to a testService2 instance that changes its
      # interdependency from testService1 to testService1B. As a result, both
      # testService2 and testService3 must be redeployed.
      # This test should succeed.

      coordinator.succeed(
          "${env} disnix-env -s ${manifestTests}/services-transitivecomposition.nix -i ${manifestTests}/infrastructure-single.nix -d ${manifestTests}/distribution-transitivecomposition.nix > result"
      )

      # Use disnix-query to check whether testService{1,1B,2,3} are
      # available on testtarget1. This test should succeed.

      coordinator.succeed(
          "${env} disnix-query -f xml ${manifestTests}/infrastructure.nix > query.xml"
      )

      coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget1']/profileManifest/services/service[name='testService1']/name\" query.xml"
      )
      coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget1']/profileManifest/services/service[name='testService1B']/name\" query.xml"
      )
      coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget1']/profileManifest/services/service[name='testService2']/name\" query.xml"
      )
      coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget1']/profileManifest/services/service[name='testService3']/name\" query.xml"
      )

      # Do an upgrade to an environment containing only one service that's a running process.
      # This test should succeed.
      coordinator.succeed(
          "${env} disnix-env -s ${manifestTests}/services-echo.nix -i ${manifestTests}/infrastructure-single.nix -d ${manifestTests}/distribution-process.nix > result"
      )

      # Do a type upgrade. We change the type of the process from 'echo' to
      # 'wrapper', triggering a redeployment. This test should succeed.
      coordinator.succeed(
          "${env} disnix-env -s ${manifestTests}/services-process.nix -i ${manifestTests}/infrastructure-single.nix -d ${manifestTests}/distribution-process.nix > result"
      )

      # Check if the 'process' has written the tmp file.
      # This test should succeed.
      testtarget1.succeed("sleep 10 && [ -f /tmp/process_out ] && rm /tmp/process_out")

      # Do an upgrade that intentionally fails.
      # This test should fail.
      coordinator.fail(
          "${env} disnix-env -s ${manifestTests}/services-fail.nix -i ${manifestTests}/infrastructure-single.nix -d ${manifestTests}/distribution-fail.nix > result"
      )

      # Check if the 'process' has written the tmp file again.
      # This test should succeed.
      testtarget1.succeed("sleep 10 && [ -f /tmp/process_out ] && rm /tmp/process_out")

      # Roll back to the previously deployed configuration
      coordinator.succeed(
          "${env} disnix-env --rollback"
      )

      # We should have one service of type echo now on the testtarget1 machine
      testtarget1.succeed(
          '[ "$(xmllint --format /nix/var/nix/profiles/disnix/default/profilemanifest.xml | grep "echo" | wc -l)" = "2" ]'
      )

      # Roll back to the first deployed configuration
      coordinator.succeed(
          "${env} disnix-env --switch-to-generation 1"
      )

      # Use disnix-query to see if the right services are installed on
      # the right target platforms. This test should succeed.

      coordinator.succeed(
          "${env} disnix-query -f xml ${manifestTests}/infrastructure.nix > query.xml"
      )

      coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget1']/profileManifest/services/service[name='testService1']/name\" query.xml"
      )
      coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget2']/profileManifest/services/service[name='testService2']/name\" query.xml"
      )
      coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget2']/profileManifest/services/service[name='testService3']/name\" query.xml"
      )

      # Test the alternative and more verbose distribution, which does
      # the same thing as the simple distribution.
      coordinator.succeed(
          "${env} disnix-env -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-alternate.nix"
      )

      # Test multi container deployment.
      coordinator.succeed(
          "${env} disnix-env -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-multicontainer.nix"
      )

      # Use disnix-query to see if the right services are installed on
      # the right target platforms. This test should succeed.

      coordinator.succeed(
          "${env} disnix-query -f xml ${manifestTests}/infrastructure.nix > query.xml"
      )

      coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget1']/profileManifest/services/service[name='testService1']/name\" query.xml"
      )
      coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget1']/profileManifest/services/service[name='testService3']/name\" query.xml"
      )
      coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget2']/profileManifest/services/service[name='testService2']/name\" query.xml"
      )
      coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget2']/profileManifest/services/service[name='testService3']/name\" query.xml"
      )

      # Remove old generation test. We remove one profile generation and we
      # check if it has been successfully removed

      oldNumOfGenerations = coordinator.succeed(
          "${env} disnix-env --list-generations | wc -l"
      )
      coordinator.succeed(
          "${env} disnix-env --delete-generations 1"
      )
      newNumOfGenerations = coordinator.succeed(
          "${env} disnix-env --list-generations | wc -l"
      )

      if int(newNumOfGenerations) == int(oldNumOfGenerations) - 1:
          print("We have successfully removed one generation!")
      else:
          raise Exception(
              "We seem to have removed more than one generation, new: {newNumOfGenerations}, old: {oldNumOfGenerations}".format(
                  newNumOfGenerations=newNumOfGenerations,
                  oldNumOfGenerations=oldNumOfGenerations,
              )
          )

      # Test disnix-capture-infra. Capture the container properties of all
      # machines and generate an infrastructure expression from it. It should
      # contain: "foo" = "bar"; twice.
      result = coordinator.succeed(
          "${env} disnix-capture-infra ${manifestTests}/infrastructure.nix | grep '\"foo\" = \"bar\"' | wc -l"
      )

      if int(result) == 2:
          print("We have foo=bar twice in the infrastructure model!")
      else:
          raise Exception(
              "We should have foo=bar twice in the infrastructure model. Instead, we have: {}".format(
                  result
              )
          )

      # It should also provide a list of supported types twice.
      result = coordinator.succeed(
          "${env} disnix-capture-infra ${manifestTests}/infrastructure.nix | grep '\"supportedTypes\" = \\[ \"process\"' | wc -l"
      )

      if int(result) == 2:
          print("We have supportedTypes twice in the infrastructure model!")
      else:
          raise Exception(
              "We should have supportedTypes twice in the infrastructure model. Instead, we have: {}".format(
                  result
              )
          )

      # Test disnix-reconstruct. Because nothing has changed the coordinator
      # profile should remain identical.

      oldNumOfGenerations = coordinator.succeed(
          "ls /nix/var/nix/profiles/per-user/root/disnix-coordinator | wc -l"
      )
      coordinator.succeed(
          "${env} disnix-reconstruct ${manifestTests}/infrastructure.nix"
      )
      newNumOfGenerations = coordinator.succeed(
          "ls /nix/var/nix/profiles/per-user/root/disnix-coordinator | wc -l"
      )

      if int(oldNumOfGenerations) == int(newNumOfGenerations):
          print("The amount of manifest generations remained the same!")
      else:
          raise Exception("The amount of manifest generations should remain the same!")

      # Test disnix-reconstruct. First, we remove the old manifests. They
      # should have been reconstructed.

      result = coordinator.succeed(
          "${env} disnix-env --delete-all-generations | wc -l"
      )

      if int(result) == 0:
          print("We have no symlinks")
      else:
          raise Exception("We should have no symlinks, instead we have: {}".format(result))

      coordinator.succeed(
          "${env} disnix-reconstruct ${manifestTests}/infrastructure.nix"
      )
      result = coordinator.succeed(
          "ls /nix/var/nix/profiles/per-user/root/disnix-coordinator | wc -l"
      )

      if int(result) == 2:
          print("We have a reconstructed manifest!")
      else:
          raise Exception("We don't have any reconstructed manifests!")

      # Use disnix-env to perform another installation combined with
      # supplemental packages.
      coordinator.succeed(
          "${env} disnix-env -s ${manifestTests}/services-complete.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-simple.nix -P ${manifestTests}/pkgs.nix"
      )

      # Use disnix-query to see if the right services are installed on
      # the right target platforms. This test should succeed.

      coordinator.succeed(
          "${env} disnix-query -f xml ${manifestTests}/infrastructure.nix > query.xml"
      )

      coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget1']/profileManifest/services/service[name='testService1']/name\" query.xml"
      )
      coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget2']/profileManifest/services/service[name='testService2']/name\" query.xml"
      )
      coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget2']/profileManifest/services/service[name='testService3']/name\" query.xml"
      )

      # Check if the packages have been properly installed as well
      testtarget1.succeed("/nix/var/nix/profiles/disnix/default/bin/curl --help")
      testtarget2.succeed("/nix/var/nix/profiles/disnix/default/bin/strace -h")

      # Use disnix-diagnose to execute a remote command
      # Check if this_component environment variable refers to testService1
      coordinator.succeed(
          "${env} disnix-diagnose -S testService1 --command 'echo $this_component' | grep testService1"
      )

      # Use disnix-diagnose to execute a remote command on a specific target
      # Check if this_component environment variable refers to testService1
      coordinator.succeed(
          "${env} disnix-diagnose -S testService1 -t testtarget1 --command 'echo $this_component' | grep testService1"
      )

      # Use disnix-diagnose to execute a remote command on a specific target and container
      # Check if this_component environment variable refers to testService1
      coordinator.succeed(
          "${env} disnix-diagnose -S testService1 -t testtarget1 -c echo --command 'echo $this_component' | grep testService1"
      )

      # Use disnix-diagnose to execute a remote command on the wrong target. This should fail.
      coordinator.fail(
          "${env} disnix-diagnose -S testService1 -t testtarget2 --command 'echo $this_component' | grep testService1"
      )

      # Deploy using a deployment model that only deploys profiles. It should have the right packages installed.
      coordinator.succeed(
          "${env} disnix-env -D ${manifestTests}/deployment.nix"
      )

      testtarget1.succeed("/nix/var/nix/profiles/disnix/default/bin/curl --help")
      testtarget2.succeed("/nix/var/nix/profiles/disnix/default/bin/strace -h")

      # Ordered service dependencies. We have one service that creates a file
      # and another that checks it contents. The latter does not know the
      # former's configuration property. It should still get activated last.

      coordinator.succeed(
          "${env} disnix-env -s ${manifestTests}/services-ordering.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-ordering.nix"
      )

      # Deploy a system with a service that provides a container. Another
      # service gets activated into that container. Check whether the exposed
      # 'hello' property has the right value in the output of the consumer
      # service.
      coordinator.succeed(
          "${env} disnix-env -s ${manifestTests}/services-containers.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-containers.nix --no-lock"
      )
      testtarget1.succeed("cat /tmp/echo_output | grep 'hello=hello-from-service-container$'")

      # The same testcase as the previous, but it uses a more verbose notation
      # in the services model.
      coordinator.succeed(
          "${env} disnix-env -s ${manifestTests}/services-containers-verbose.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-containers.nix --no-lock"
      )
      testtarget1.succeed("cat /tmp/echo_output | grep 'hello=hello-from-service-container$'")

      # Testcase that works with an echo container as part of the infrastructure.
      # It checks whether the exposed 'hello' property matches what we expect.
      coordinator.succeed(
          "${env} disnix-env -s ${manifestTests}/services-infracontainer.nix -i ${manifestTests}/infrastructure-container.nix -d ${manifestTests}/distribution-infracontainer.nix --no-lock"
      )
      testtarget1.succeed(
          "cat /var/log/disnix/* | grep 'hello=hello-from-infrastructure-container$'"
      )

      # Testcase that deploys a parametrized services model. We should get a
      # service deployed that has a different name (myService) instead of the
      # default (testService)
      coordinator.succeed(
          "${env} disnix-env -s ${manifestTests}/services-parametrized.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-parametrized.nix --extra-params '{ prefix = \"myService\"; }'"
      )
      coordinator.succeed(
          "${env} disnix-query -f xml ${manifestTests}/infrastructure.nix > query.xml"
      )
      result = coordinator.succeed(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget1']/profileManifest/services/service[name='testPrefixService']/pkg\" query.xml"
      )

      if "-myService" in result:
          print("The result contains myService!")
      else:
          raise Exception("The result should contain: myService")

      # Testcase the undeploys everything.
      coordinator.succeed(
          "${env} disnix-env --undeploy -i ${manifestTests}/infrastructure-container.nix"
      )
      coordinator.succeed(
          "${env} disnix-query -f xml ${manifestTests}/infrastructure-container.nix > query.xml"
      )
      coordinator.fail(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget1']/profileManifest/services/*\" query.xml"
      )
      coordinator.fail(
          "xmllint --xpath \"/profileManifestTargets/target[@name='testtarget2']/profileManifest/services/*\" query.xml"
      )

      # Deploy a stateful service that relies on a container provided by a service
      coordinator.succeed(
          "${env} disnix-env -s ${manifestTests}/services-containers-state.nix -i ${manifestTests}/infrastructure.nix -d ${manifestTests}/distribution-containers-state.nix"
      )

      # Undeploy the system. This should fail, because undeploying the container
      # provider service prevents the stateful service's state from being
      # captured.
      coordinator.fail(
          "${env} disnix-env --undeploy -i ${manifestTests}/infrastructure.nix"
      )

      # Undeploying the system without state migration is allowed.
      coordinator.succeed(
          "${env} disnix-env --undeploy -i ${manifestTests}/infrastructure.nix --no-migration"
      )
    '';
}
