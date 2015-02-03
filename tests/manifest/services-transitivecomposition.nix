{distribution, system, pkgs}:

let
  customPkgs = import ./pkgs { inherit pkgs system; };
in
rec {
  testService1 = {
    name = "testService1";
    pkg = customPkgs.testService1;
    type = "echo";
  };
  
  testService1B = {
    name = "testService1B";
    pkg = customPkgs.testService1B;
    type = "echo";
  };
  
  testService2 = {
    name = "testService2";
    pkg = customPkgs.testService2;
    dependsOn = {
      testService1 = testService1B; # A custom composition
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
