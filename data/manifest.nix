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
