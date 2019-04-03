let
  generateManifestFromDeploymentModel =
    {manifest, pkgs}:

    let
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
    };

  generateManifestFromArchitectureFun =
    { architectureFun
    , nixpkgs
    , pkgs
    , clientInterface
    , targetProperty
    , deployState
    }:

    let
      normalizeArchitecture = import ./normalize.nix {
        inherit (pkgs) lib;
      };

      generateManifest = import ./generate-manifest.nix {
        inherit pkgs;
        inherit (pkgs) lib;
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
    in
    generateManifestFromDeploymentModel {
      inherit manifest pkgs;
    };
in
{
  generateManifestFromArchitectureModel =
    { architectureFile
    , targetProperty
    , clientInterface
    , deployState ? false
    , nixpkgs ? <nixpkgs>
    }:

    let
      architectureFun = import architectureFile;
      pkgs = import nixpkgs {};
    in
    generateManifestFromArchitectureFun {
      inherit architectureFun;
      inherit nixpkgs pkgs targetProperty clientInterface deployState;
    };

  generateManifestFromModels =
    { servicesFile ? null
    , infrastructureFile
    , distributionFile ? null
    , packagesFile ? null
    , targetProperty
    , clientInterface
    , deployState ? false
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
        inherit servicesFun infrastructure distributionFun packagesFun;
      };
    in
    generateManifestFromArchitectureFun {
      inherit architectureFun;
      inherit nixpkgs pkgs targetProperty clientInterface deployState;
    };

  generateManifestFromDeploymentModel =
    { deploymentFile
    , nixpkgs ? <nixpkgs>
    }:

    generateManifestFromDeploymentModel {
      manifest = import deploymentFile;
      pkgs = import nixpkgs {};
    };
}
