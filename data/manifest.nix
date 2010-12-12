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
  lib = import ./lib.nix;
  manifest = lib.generateManifest pkgs servicesFun infrastructure distributionFun targetProperty;
  
  generateManifestXSL = ./generatemanifest.xsl;
in
pkgs.stdenv.mkDerivation {
  name = "manifest.xml";
  buildInputs = [ pkgs.libxslt ];
  buildCommand = ''
    (
    cat <<EOF
    ${builtins.toXML manifest}
    EOF
    ) | xsltproc ${generateManifestXSL} - > $out
  '';
}
