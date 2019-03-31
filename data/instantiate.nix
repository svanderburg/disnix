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

  wrapArchitecture = import ./wrap-architecture.nix {
    inherit (pkgs) lib;
  };

  normalizeArchitecture = import ./normalize.nix {
    inherit (pkgs) lib;
  };

  generateDistributedDerivation = import ./generate-distributed-derivation.nix {
    inherit (pkgs) lib;
  };

  architectureFun = wrapArchitecture {
    inherit servicesFun infrastructure distributionFun;
  };

  architecture = normalizeArchitecture {
    inherit architectureFun nixpkgs;
    defaultClientInterface = clientInterface;
    defaultTargetProperty = targetProperty;
    defaultDeployState = false;
  };

  distributedDerivation = generateDistributedDerivation {
    inherit architecture;
  };

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
