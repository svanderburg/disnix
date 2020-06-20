{lib}:

let
  inherit (builtins) attrNames listToAttrs;

  normalizeInfrastructure = import ./normalize-infrastructure.nix {
    inherit lib;
  };

  normalizeServices = import ./normalize-services.nix {
    inherit lib;
  };

  generateTargetPackages = import ./generate-target-packages.nix {
    inherit lib;
  };

in
rec {
  /*
   * Appends the packages provided by the services to the packages that were already
   * declared in the deployment architecture model.
   */
  appendTargetPackages = {architecture, normalizedInfrastructure, normalizedServices}:
    let
      targetPackages = generateTargetPackages.generateTargetPackagesFromServices {
        inherit normalizedInfrastructure normalizedServices;
      };
    in
    if (architecture ? targetPackages)
    then
      let
        allTargetNames = attrNames (architecture.targetPackages // targetPackages);
      in
      {
        targetPackages = listToAttrs (map (targetName:
          { name = targetName;
            value = architecture.targetPackages."${targetName}" or []
              ++ targetPackages."${targetName}" or [];
          }
        ) allTargetNames);
      }
    else {
      inherit targetPackages;
    };

  /*
   * Normalizes the deployment architecture model by:
   * - Augmenting all targets and services with default properties
   * - Turning all references to services and targets into reference specifications
   * - Building all relevant packages for each system architecture
   */
  generateNormalizedDeploymentArchitecture = {architectureFun, nixpkgs, defaultClientInterface, defaultTargetProperty, defaultDeployState, extraParams}:
    let
      architecture = architectureFun ({
        pkgs = import nixpkgs {};
        system = null;
      } // extraParams);

      normalizedInfrastructure = normalizeInfrastructure.normalizeInfrastructure {
        inherit (architecture) infrastructure;
        inherit defaultClientInterface defaultTargetProperty;
      };

      targetAliases = normalizeInfrastructure.generateTargetAliases {
        inherit normalizedInfrastructure;
      };

      normalizedServices = normalizeServices.normalizeServices {
        inherit (architecture) services;
        inherit defaultDeployState defaultTargetProperty normalizedInfrastructure targetAliases architectureFun extraParams nixpkgs;
      };
    in
    architecture // {
      infrastructure = normalizedInfrastructure;
      services = normalizedServices;
    } // appendTargetPackages {
      inherit architecture normalizedInfrastructure normalizedServices;
    };
}
