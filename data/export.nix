{ servicesFile
, infrastructureFile
, distributionFile
, targetProperty
}:

let 
  servicesFun = import servicesFile;
  infrastructure = import infrastructureFile;
  distributionFun = import distributionFile;
  
  pkgs = import (builtins.getEnv "NIXPKGS_ALL") {};
  lib = import ./lib.nix;
  distributionExport = lib.generateDistributionExport pkgs servicesFun infrastructure distributionFun targetProperty;
in
pkgs.stdenv.mkDerivation {
  name = "distributionExport.xml";
  buildCommand = ''
    cat > $out <<EOF
    ${builtins.toXML distributionExport}
    EOF
  '';
}
