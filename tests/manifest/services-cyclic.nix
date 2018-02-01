{distribution, invDistribution, system, pkgs}:

let
  customPkgs = import ./pkgs { inherit pkgs system; };
in
rec {
  cyclicTestService1 = {
    name = "cyclicTestService1";
    pkg = customPkgs.cyclicTestService1;
    type = "echo";
    connectsTo = {
      inherit cyclicTestService2;
    };
  };
  
  cyclicTestService2 = {
    name = "cyclicTestService2";
    pkg = customPkgs.cyclicTestService2;
    connectsTo = {
      inherit cyclicTestService1;
    };
    type = "echo";
  };
}
