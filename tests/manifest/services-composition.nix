{distribution, system}:

let
  pkgs = import ./pkgs { inherit system; };
in
rec {
  testService1 = {
    name = "testService1";
    pkg = pkgs.testService1;
    type = "echo";
  };
  
  testService2 = {
    name = "testService2";
    pkg = pkgs.testService2;
    dependsOn = {
      inherit testService1;
    };
    type = "echo";
  };

  testService2B = {
    name = "testService2B";
    pkg = pkgs.testService2B;
    dependsOn = {
      inherit testService1;
    };
    type = "echo";
  };
  
  testService3 = {
    name = "testService3";
    pkg = pkgs.testService3;
    dependsOn = {
      inherit testService1;
      testService2 = testService2B; # A custom composition
    };
    type = "echo";
  };
}
