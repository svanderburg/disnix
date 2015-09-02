{distribution, invDistribution, system, pkgs}:

let
  dysnomia = builtins.storePath (builtins.getEnv "dysnomia");
  
  wrapper = import ./wrapper.nix {
    inherit dysnomia;
    inherit (pkgs) stdenv;
  };
in
rec {
  testService1 = rec {
    name = "testService1";
    pkg = wrapper { inherit name; };
    dependsOn = {};
    type = "wrapper";
    deployState = true;
  };
  
  testService2 = rec {
    name = "testService2";
    pkg = wrapper { inherit name; };
    dependsOn = {};
    type = "wrapper";
    deployState = true;
  };
}
