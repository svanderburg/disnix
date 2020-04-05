{distribution, invDistribution, system, pkgs}:

let
  customPkgs = import ./pkgs { inherit pkgs system; };
in
{
  testServiceContainerConsumer = {
    name = "testServiceContainerConsumer";
    pkg = customPkgs.testServiceContainerConsumer;
    type = "echo";
  };
}
