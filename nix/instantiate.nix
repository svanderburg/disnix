let
  generateDistributedDerivationFromBuildModel =
    { distributedDerivation, pkgs }:

    let
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
    };

  generateDistributedDerivationFromArchitectureFun =
    { architectureFun
    , nixpkgs
    , pkgs
    , clientInterface
    , targetProperty
    , extraParams
    }:

    let
      normalizeArchitecture = import ./normalize-architecture.nix {
        inherit (pkgs) lib;
      };

      generateDistributedDerivation = import ./generate-distributed-derivation.nix {
        inherit (pkgs) lib;
      };

      architecture = normalizeArchitecture {
        inherit architectureFun nixpkgs extraParams;
        defaultClientInterface = clientInterface;
        defaultTargetProperty = targetProperty;
        defaultDeployState = false;
      };

      distributedDerivation = generateDistributedDerivation {
        inherit architecture;
      };
    in
    generateDistributedDerivationFromBuildModel {
      inherit distributedDerivation pkgs;
    };
in
{
  generateDistributedDerivationFromArchitectureModel =
    { architectureFile
    , targetProperty
    , clientInterface
    , extraParams ? {}
    , nixpkgs ? <nixpkgs>
    }:

    let
      architectureFun = import architectureFile;
      pkgs = import nixpkgs {};
    in
    generateDistributedDerivationFromArchitectureFun {
      inherit architectureFun;
      inherit nixpkgs pkgs targetProperty clientInterface extraParams;
    };

  generateDistributedDerivationFromModels =
    { servicesFile ? null
    , infrastructureFile
    , distributionFile ? null
    , packagesFile ? null
    , targetProperty
    , clientInterface
    , extraParams ? {}
    , nixpkgs ? <nixpkgs>
    }:

    let
      servicesFun = if servicesFile == null then null else import servicesFile;
      infrastructure = import infrastructureFile;
      distributionFun = if distributionFile == null then null else import distributionFile;
      packagesFun = if packagesFile == null then null else import packagesFile;

      pkgs = import nixpkgs {};

      wrapArchitecture = import ./wrap-architecture.nix {
        inherit (pkgs) lib;
      };

      architectureFun = wrapArchitecture {
        inherit servicesFun infrastructure distributionFun packagesFun extraParams;
      };
    in
    generateDistributedDerivationFromArchitectureFun {
      inherit architectureFun;
      inherit nixpkgs pkgs targetProperty clientInterface;
      extraParams = {};
    };

  generateDistributedDerivationFromBuildModel =
    { buildFile
    , nixpkgs ? <nixpkgs>
    }:

    generateDistributedDerivationFromBuildModel {
      distributedDerivation = import buildFile;
      pkgs = import nixpkgs {};
    };
}
