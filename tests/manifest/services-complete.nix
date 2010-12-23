{distribution, system, pkgs}:

let
  customPkgs = import ./pkgs { inherit system; };
in
rec {
  testService1 = {
    name = "testService1";
    pkg = customPkgs.testService1;
    type = "echo";
  };
  
  testService2 = {
    name = "testService2";
    pkg = customPkgs.testService2;
    dependsOn = {
      inherit testService1;
    };
    type = "echo";
  };
  
  testService3 = {
    name = "testService3";
    pkg = customPkgs.testService3;
    dependsOn = {
      inherit testService1 testService2;
    };
    type = "echo";
  };
}
