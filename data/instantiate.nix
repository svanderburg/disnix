{ servicesFile
, infrastructureFile
, distributionFile
, targetProperty
, nixpkgs ? builtins.getEnv "NIXPKGS_ALL"
}:

let 
  servicesFun = import servicesFile;
  infrastructure = import infrastructureFile;
  distributionFun = import distributionFile;
  
  pkgs = import nixpkgs {};
  lib = import ./lib.nix { inherit nixpkgs pkgs; };
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
