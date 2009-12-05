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
  
  generateDistributionExportXSL = ./generatedistributionexport.xsl;
in
pkgs.stdenv.mkDerivation {
  name = "distributionExport.xml";
  buildInputs = [ pkgs.libxslt ];
  buildCommand = ''
    (
    cat <<EOF
    ${builtins.toXML distributionExport}
    EOF
    ) | xsltproc ${generateDistributionExportXSL} - > $out
  '';
}
