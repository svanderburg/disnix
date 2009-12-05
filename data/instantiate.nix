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
  distributedDerivation = lib.generateDistributedDerivation servicesFun infrastructure distributionFun targetProperty;
  
  generateDistributedDerivationXSL = ./generatedistributedderivation.xsl;
in
pkgs.stdenv.mkDerivation {
  name = "distributedDerivation.xml";
  buildInputs = [ pkgs.libxslt ];
  
  buildCommand = ''
    (
    cat <<EOF
    ${builtins.toXML distributedDerivation}
    EOF
    ) | xsltproc ${generateDistributedDerivationXSL} - > $out
  '';
}
