{distribution, invDistribution, system, pkgs}:

let
  customPkgs = import ./pkgs { inherit pkgs system; };
in
{
  testServiceContainerProvider = {
    name = "testServiceContainerProvider";
    hello = "hello-from-service-container"; # Property that is deliberately different from the container on infrastructure level
    pkg = customPkgs.testServiceContainerProvider;
    providesContainer = "echo"; # Expose an echo container and takes all non-system properties from the service as container properties
    type = "wrapper";
  };

  testServiceContainerConsumer = {
    name = "testServiceContainerConsumer";
    pkg = customPkgs.testServiceContainerConsumer;
    type = "echo";
    deployState = true; # Make it a stateful component so that it relies on the presence of testServiceContainerProvide during the datamigration phase
  };
}
