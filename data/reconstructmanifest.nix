{ infrastructureFile
, capturedPropertiesFile
, targetProperty
, clientInterface
, deployState ? false
, nixpkgs ? <nixpkgs>
}:

let
  pkgs = import nixpkgs {};

  infrastructure = import infrastructureFile;
  capturedProperties = import capturedPropertiesFile;

  reconstructManifest = import ./reconstruct.nix;

  reconstructedManifest = reconstructManifest {
    inherit infrastructure capturedProperties;
    defaultTargetProperty = targetProperty;
    defaultClientInterface = clientInterface;
  };

  generateManifestXSL = ./generatemanifest.xsl;
in
pkgs.stdenv.mkDerivation {
  name = "manifest.xml";
  buildInputs = [ pkgs.libxslt ];
  manifestXML = builtins.toXML reconstructedManifest;
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
