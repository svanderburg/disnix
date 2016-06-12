{pkgs, system, distribution, invDistribution}:

pkgs.lib.mapAttrs (name: targets:
  let
    attrPath = pkgs.lib.splitString "." name;
  in
  { inherit name;
    pkg = pkgs.lib.attrByPath attrPath (throw "package: ${name} cannot be referenced in the package set") pkgs;
    type = "package";
  }
) distribution
