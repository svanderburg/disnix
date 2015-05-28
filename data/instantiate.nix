{ servicesFile
, infrastructureFile
, distributionFile
, targetProperty
, clientInterface
, nixpkgs ? <nixpkgs>
}:

let 
  servicesFun = import servicesFile;
  infrastructure = import infrastructureFile;
  distributionFun = import distributionFile;
  
  pkgs = import nixpkgs {};
  lib = import ./lib.nix { inherit nixpkgs pkgs; };
  distributedDerivation = lib.generateDistributedDerivation servicesFun infrastructure distributionFun targetProperty clientInterface;
  
  generateDistributedDerivationXSL = ./generatedistributedderivation.xsl;
in
pkgs.stdenv.mkDerivation {
  name = "distributedDerivation.xml";
  buildInputs = [ pkgs.libxslt ];
  distributedDerivationXML = builtins.toXML distributedDerivation;
  passAsFile = [ "distributedDerivationXML" ];
  
  buildCommand = ''
    if [ "$distributedDerivationXMLPath" != "" ]
    then
        xsltproc ${generateDistributedDerivationXSL} $distributedDerivationXMLPath > $out
    else
    (
    cat <<EOF
    $distributedDerivationXML
    EOF
    ) | xsltproc ${generateDistributedDerivationXSL} - > $out
    fi
  '';
}
