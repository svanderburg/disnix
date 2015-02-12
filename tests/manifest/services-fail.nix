{distribution, system, pkgs}:

let
  customPkgs = import ./pkgs { inherit pkgs system; };
in
rec {
  fail = {
    name = "fail";
    pkg = customPkgs.fail;
    type = "wrapper";
  };
}
