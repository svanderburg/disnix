{pkgs, lib}:

let
  inherit (builtins) attrNames concatLists filter getAttr hashString listToAttrs stringLength substring toXML unsafeDiscardStringContext;

  generateProfiles = import ./generate-profiles.nix {
    inherit pkgs lib;
  };
in
rec {
  /*
   * Generates a SHA256 code to uniquely identify a service deployment for a
   * paricular system architecture.
   */
  generateHash = {manifestService}:
    unsafeDiscardStringContext (hashString "sha256" (toXML {
      inherit (manifestService) name type pkg dependsOn;
    }));

  /*
   * Translates an attribute set of dependency parameters into dependency mappings.
   *
   * Example:
   *   { foo1 = { service = "foo"; targets = [ { target = "test1"; container = "process"; } ]; };
   *     bar1 = { service = "bar"; targets = [ { target = "test2"; container = "process"; } ]; };
   *   }
   *   =>
   *   [ { service = "hash1..."; target = "test1"; container = "process"; }
   *     { service = "hash2..."; target = "test2"; container = "process"; }
   *   ]
   */
  generateDependencyMappings = {dependencyParams, normalizedServices, normalizedInfrastructure}:
    concatLists (map (paramName:
      let
        dependencyReference = getAttr paramName dependencyParams;
        normalizedDependency = getAttr dependencyReference.service normalizedServices;
      in
      map (targetReference:
        let
          manifestDependency = generateManifestServiceFromServiceForArchitecture {
            inherit normalizedServices normalizedInfrastructure;
            normalizedService = normalizedDependency;
            system = normalizedInfrastructure."${targetReference.target}".system;
          };
        in
        {
          service = generateHash {
            manifestService = manifestDependency;
          };

          inherit (targetReference) target container;
        }
      ) (dependencyReference.targets or [])
    ) (attrNames dependencyParams));

  /*
   * Translates a service in the normalized architecture model to a service in
   * the deployment model for a particular system architecture.
   *
   * Example:
   *   { name = "foo";
   *     pkgs.x86_64-linux = "/nix/store/aaaa...-foo";
   *     dependsOn = {
   *       bar1 = { service = "bar"; targets = [ { container = "process"; target = "test2"; } ]; }
   *     };
   *     targets = [
   *       { target = "test1"; container = "process"; } # An x86_64-linux machine
   *     ];
   *   }
   *   =>
   *   {
   *     name = "foo";
   *     pkg = "/nix/store/aaaa...-foo";
   *     dependsOn = [
   *       { service = "hash2..."; target = "test2"; container = "process"; }
   *     ];
   *     ...
   *   }
   */
  generateManifestServiceFromServiceForArchitecture = {normalizedService, normalizedServices, system, normalizedInfrastructure}:
    let
      orderedDependencies = normalizedService.activatesAfter or {} // normalizedService.dependsOn or {};
    in
    {
      inherit (normalizedService) name type;
      pkg = (getAttr system normalizedService.pkgs).outPath;

      dependsOn = generateDependencyMappings {
        inherit normalizedServices normalizedInfrastructure;
        dependencyParams = orderedDependencies;
      };
      connectsTo = generateDependencyMappings {
        inherit normalizedServices normalizedInfrastructure;
        dependencyParams = normalizedService.connectsTo or {};
      };
    } // lib.optionalAttrs (normalizedService ? providesContainers) {
      inherit (normalizedService) providesContainers;
    };

  /*
   * Generates one or more services for the deployment model from a service in
   * the normalized architecture model.
   *
   * A service can be translated into multiple manifest services, if the former
   * is mapped to two (or more) machines having a different system architecture.
   *
   * Example:
   *   { name = "foo";
   *     pkgs = {
   *       x86_64-linux = "/nix/store/aaaa...-foo";
   *       x86_64-darwin = "/nix/store/bbbb...-foo";
   *     };
   *     targets = [
   *       { target = "test1"; container = "process"; } # An x86_64-linux machine
   *       { target = "test2"; container = "process"; } # An x86_64-darwin machine
   *     ];
   *   }
   *   =>
   *   [
   *     { name = "hash1...";
   *       value = {
   *         name = "foo";
   *         pkg = "/nix/store/aaaa...-foo";
   *         ...
   *       };
   *     }
   *
   *     { name = "hash2...";
   *       value = {
   *         name = "foo";
   *         pkg = "/nix/store/bbbb...-foo";
   *         ...
   *       };
   *     }
   *   ]
   */
  generateManifestServicesFromService = {normalizedService, normalizedServices, normalizedInfrastructure}:
    map (system:
      let
        manifestService = generateManifestServiceFromServiceForArchitecture {
          inherit normalizedService normalizedServices system normalizedInfrastructure;
        };
      in
      { name = generateHash {
          inherit manifestService;
        };
        value = manifestService;
      }
    ) (attrNames normalizedService.pkgs);

  /*
   * Returns the names for all services that do not have the package type.
   *
   * Example:
   *   { foo = { name = "foo"; type = "process"; };
   *     bar = { name = "bar"; type = "package"; };
   *   }
   *   =>
   *   [ "foo" ]
   */
  generateNonPackageServiceNames = {normalizedServices}:
    filter (serviceName:
      let
        service = getAttr serviceName normalizedServices;
      in
      service.type != "package"
    ) (attrNames normalizedServices);

  /*
   * Generates for all services in a normalized service architecture, services
   * for the deployment model, that can be used to provide a one-on-one mapping
   * between deployment artifacts and containers on a target machine.
   */
  generateManifestServices = {normalizedServices, normalizedInfrastructure}:
    let
      nonPackageServiceNames = generateNonPackageServiceNames {
        inherit normalizedServices;
      };
    in
    listToAttrs (concatLists (map (serviceName:
      let
        normalizedService = getAttr serviceName normalizedServices;
      in
      generateManifestServicesFromService {
        inherit normalizedService normalizedServices normalizedInfrastructure;
      }
    ) (attrNames normalizedServices)));

  /*
   * Generates for all services in the normalized architecture model that are
   * mapped to containers on target machines, one-on-one mappings between the
   * deployment artifacts and deployment targets.
   *
   * Example:
   *   { foo = {
   *       name = "foo";
   *       targets = [
   *         { target = "target1"; container = "process"; }
   *         { target = "target2"; container = "process"; }
   *         { target = "target2"; container = "mycontainer"; containerProvidedByService = "containerprovider"; }
   *       ];
   *       ...
   *     };
   *   }
   *   =>
   *   [
   *     { service = "hash1..."; target = "target1"; container = "process"; }
   *     { service = "hash2..."; target = "target2"; container = "process"; }
   *     { service = "hash3..."; target = "target2"; container = "mycontainer"; containerProvidedByService = "hash4..."; }
   *   ]
   */
  generateServiceMappings = {normalizedServices, normalizedInfrastructure}:
    let
      nonPackageServiceNames = generateNonPackageServiceNames {
        inherit normalizedServices;
      };
    in
    concatLists (map (serviceName:
      let
        normalizedService = getAttr serviceName normalizedServices;
      in
      map (targetReference:
        let
          normalizedTarget = getAttr targetReference.target normalizedInfrastructure;
        in
        targetReference // {
          service = generateHash {
            manifestService = generateManifestServiceFromServiceForArchitecture {
              inherit normalizedService normalizedServices normalizedInfrastructure;
              inherit (normalizedTarget) system;
            };
          };
        } // lib.optionalAttrs (targetReference ? containerProvidedByService) {
          containerProvidedByService = generateHash {
            manifestService = generateManifestServiceFromServiceForArchitecture {
              normalizedService = getAttr targetReference.containerProvidedByService normalizedServices;
              inherit normalizedServices normalizedInfrastructure;
              inherit (normalizedTarget) system;
            };
          };
        }
      ) normalizedService.targets or []
    ) nonPackageServiceNames);

  /*
   * Generates for all stateful services in the normalized architecture model
   * (that have a property deployState = true) that are mapped to containers on
   * target machines, one-on-one mappings between the
   * deployment artifacts and deployment targets.
   *
   * Example:
   *   { foo = {
   *       name = "foo";
   *       targets = [
   *         { target = "target1"; container = "process"; }
   *         { target = "target2"; container = "process"; }
   *       ];
   *       ...
   *     };
   *   }
   *   =>
   *   [
   *     { service = "hash1..."; target = "target1"; container = "process"; component = "foo"; }
   *     { service = "hash2..."; target = "target2"; container = "process"; component = "foo"; }
   *   ]
   */
  generateSnapshotMappings = {normalizedServices, normalizedInfrastructure}:
    let
      statefulServiceNames = filter (serviceName:
        let
          normalizedService = getAttr serviceName normalizedServices;
        in
        normalizedService.deployState
      ) (attrNames normalizedServices);
    in
    concatLists (map (serviceName:
      let
        normalizedService = getAttr serviceName normalizedServices;
      in
      map (targetReference:
        let
          normalizedTarget = getAttr targetReference.target normalizedInfrastructure;
          pkg = normalizedService.pkgs."${normalizedTarget.system}".outPath;
        in
        targetReference // {
          service = generateHash {
            manifestService = generateManifestServiceFromServiceForArchitecture {
              inherit normalizedService normalizedServices normalizedInfrastructure;
              inherit (normalizedTarget) system;
            };
          };
          component = substring 33 (stringLength pkg) (baseNameOf pkg);
        } // lib.optionalAttrs (targetReference ? containerProvidedByService) {
          containerProvidedByService = generateHash {
            manifestService = generateManifestServiceFromServiceForArchitecture {
              normalizedService = getAttr targetReference.containerProvidedByService normalizedServices;
              inherit normalizedServices normalizedInfrastructure;
              inherit (normalizedTarget) system;
            };
          };
        }
      ) normalizedService.targets or []
    ) statefulServiceNames);

  /*
   * Generates a deployment model from a normalized architecture model.
   * The deployment model defines the packages to be mapped to target machines,
   * the available services per system architecture, the mapping of services
   * to machines and the mapping of snapshots to machines
   */
  generateDeploymentModel = {normalizedArchitecture}:
    let
      deploymentModel = {
        inherit (normalizedArchitecture) infrastructure;

        profiles = generateProfiles.generateProfiles {
          inherit (normalizedArchitecture) targetPackages;
          inherit deploymentModel;
        };

        services = generateManifestServices {
          normalizedServices = normalizedArchitecture.services;
          normalizedInfrastructure = normalizedArchitecture.infrastructure;
        };

        serviceMappings = generateServiceMappings {
          normalizedServices = normalizedArchitecture.services;
          normalizedInfrastructure = normalizedArchitecture.infrastructure;
        };

        snapshotMappings = generateSnapshotMappings {
          normalizedServices = normalizedArchitecture.services;
          normalizedInfrastructure = normalizedArchitecture.infrastructure;
        };
      };
    in
    deploymentModel;
}
