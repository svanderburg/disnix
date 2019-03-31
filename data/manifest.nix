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

  wrapArchitecture = import ./wrap-architecture.nix {
    inherit (pkgs) lib;
  };

  normalizeArchitecture = import ./normalize.nix {
    inherit (pkgs) lib;
  };

  generateManifest = import ./generate-manifest.nix {
    inherit pkgs;
    inherit (pkgs) lib;
  };

  architectureFun = wrapArchitecture {
    inherit servicesFun infrastructure distributionFun;
  };

  architecture = normalizeArchitecture {
    inherit architectureFun nixpkgs;
    defaultClientInterface = clientInterface;
    defaultTargetProperty = targetProperty;
    defaultDeployState = deployState;
  };

  manifest = generateManifest {
    inherit architecture;
  };

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
