{distribution, invDistribution, system, pkgs, prefix ? "testPrefixService"}:

let
  customPkgs = import ./pkgs { inherit pkgs system; };
in
rec {
  testPrefixService = {
    name = "testPrefixService";
    pkg = customPkgs.testPrefixService {
      inherit prefix;
    };
    type = "echo";
  };
}
