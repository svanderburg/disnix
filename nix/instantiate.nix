rec {
  generateDistributedDerivationXMLFromBuildModel = {buildModel, pkgs}:
    let
      generateDistributedDerivationXSL = ./generatedistributedderivation.xsl;
    in
    pkgs.stdenv.mkDerivation {
      name = "distributedderivation.xml";
      buildInputs = [ pkgs.libxslt ];
      distributedDerivationXML = builtins.toXML buildModel;
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

      generateBuildModel = import ./generate-build-model.nix {
        inherit (pkgs) lib;
        inherit pkgs;
      };

      normalizedArchitecture = normalizeArchitecture.generateNormalizedDeploymentArchitecture {
        inherit architectureFun nixpkgs extraParams;
        defaultClientInterface = clientInterface;
        defaultTargetProperty = targetProperty;
        defaultDeployState = false;
      };

      buildModel = generateBuildModel.generateBuildModel {
        inherit normalizedArchitecture;
      };
    in
    generateDistributedDerivationXMLFromBuildModel {
      inherit buildModel pkgs;
    };

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
      inherit architectureFun nixpkgs pkgs targetProperty clientInterface extraParams;
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

      architectureFun = wrapArchitecture.wrapBasicInputsModelsIntoArchitectureFun {
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

    generateDistributedDerivationXMLFromBuildModel {
      buildModel = import buildFile;
      pkgs = import nixpkgs {};
    };
}
