{distribution, system, pkgs}:

let
  customPkgs = import ./pkgs { inherit pkgs system; };
in
rec {
  process = {
    name = "process";
    pkg = customPkgs.process;
    type = "echo";
  };
}
