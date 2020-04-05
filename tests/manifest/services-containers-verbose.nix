{distribution, invDistribution, system, pkgs}:

let
  customPkgs = import ./pkgs { inherit pkgs system; };
in
{
  testServiceContainerProvider = {
    name = "testServiceContainerProvider";
    pkg = customPkgs.testServiceContainerProvider;
    type = "wrapper";

    providesContainers = {
      echo = {
        hello = "hello-from-service-container"; # Property that is deliberately different from the container on infrastructure level
      };

      foobar = {
        value = "This is not used";
      };
    };
  };

  testServiceContainerConsumer = {
    name = "testServiceContainerConsumer";
    pkg = customPkgs.testServiceContainerConsumer;
    type = "echo";
  };
}
