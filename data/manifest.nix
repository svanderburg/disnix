{ servicesFile
, infrastructureFile
, distributionFile
, targetProperty
, clientInterface
, deployState ? false
, nixpkgs ? <nixpkgs>
}:

let 
  servicesFun = import servicesFile;
  infrastructure = import infrastructureFile;
  distributionFun = import distributionFile;
  
  pkgs = import nixpkgs {};
  lib = import ./lib.nix { inherit nixpkgs pkgs; };
  manifest = lib.generateManifest pkgs servicesFun infrastructure distributionFun targetProperty clientInterface deployState;
  
  generateManifestXSL = ./generatemanifest.xsl;
in
pkgs.stdenv.mkDerivation {
  name = "manifest.xml";
  buildInputs = [ pkgs.libxslt ];
  manifestXML = builtins.toXML manifest;
  passAsFile = [ "manifestXML" ];
  
  buildCommand = ''
    if [ "$manifestXMLPath" != "" ]
    then
        xsltproc ${generateManifestXSL} $manifestXMLPath > $out
    else
    (
    cat <<EOF
    $manifestXML
    EOF
    ) | xsltproc ${generateManifestXSL} - > $out
    fi
  '';
}
