rec {
  generateManifestXMLFromDeploymentModel = {deploymentModel, pkgs}:
    let
      generateManifestXSL = ./generatemanifest.xsl;
    in
    pkgs.stdenv.mkDerivation {
      name = "manifest.xml";
      buildInputs = [ pkgs.libxslt ];
      manifestXML = builtins.toXML deploymentModel;
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
    };

  generateManifestFromArchitectureFun =
    { architectureFun
    , nixpkgs ? <nixpkgs>
    , pkgs
    , clientInterface
    , targetProperty
    , deployState
    , extraParams ? {}
    }:

    let
      normalizeArchitecture = import ./normalize-architecture.nix {
        inherit (pkgs) lib;
      };

      generateDeploymentModel = import ./generate-deployment-model.nix {
        inherit pkgs;
        inherit (pkgs) lib;
      };

      normalizedArchitecture = normalizeArchitecture.generateNormalizedDeploymentArchitecture {
        inherit architectureFun nixpkgs extraParams;
        defaultClientInterface = clientInterface;
        defaultTargetProperty = targetProperty;
        defaultDeployState = deployState;
      };

      deploymentModel = generateDeploymentModel.generateDeploymentModel {
        inherit normalizedArchitecture;
      };
    in
    generateManifestXMLFromDeploymentModel {
      inherit deploymentModel pkgs;
    };

  generateManifestFromArchitectureModel =
    { architectureFile
    , targetProperty
    , clientInterface
    , deployState ? false
    , extraParams ? {}
    , nixpkgs ? <nixpkgs>
    }:

    let
      architectureFun = import architectureFile;
      pkgs = import nixpkgs {};
    in
    generateManifestFromArchitectureFun {
      inherit architectureFun;
      inherit nixpkgs pkgs targetProperty clientInterface deployState extraParams;
    };

  generateManifestFromModels =
    { servicesFile ? null
    , infrastructureFile
    , distributionFile ? null
    , packagesFile ? null
    , targetProperty
    , clientInterface
    , deployState ? false
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
    generateManifestFromArchitectureFun {
      inherit architectureFun;
      inherit nixpkgs pkgs targetProperty clientInterface deployState;
      extraParams = {};
    };

  generateManifestFromDeploymentModel =
    { deploymentFile
    , nixpkgs ? <nixpkgs>
    }:

    generateManifestXMLFromDeploymentModel {
      deploymentModel = import deploymentFile;
      pkgs = import nixpkgs {};
    };

  generateUndeploymentManifest =
    { infrastructureFile
    , nixpkgs ? <nixpkgs>
    , targetProperty
    , clientInterface
    , deployState ? false
    }:

    let
      pkgs = import nixpkgs {};
    in
    generateManifestFromArchitectureFun {
      architectureFun =
        {system, pkgs}:

        {
          services = {};
          infrastructure = import infrastructureFile;
        };

      inherit nixpkgs pkgs targetProperty clientInterface deployState;
      extraParams = {};
    };
}
