{distribution, invDistribution, system, pkgs}:

let
  customPkgs = import ./pkgs { inherit pkgs system; };
in
rec {
  process = {
    name = "process";
    pkg = customPkgs.process;
    type = "wrapper";
  };

  afterProcess = {
    name = "afterProcess";
    pkg = customPkgs.afterProcess;
    activatesAfter = {
      inherit process;
    };
    type = "wrapper";
  };
}
