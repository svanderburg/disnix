{lib}:

let
  inherit (builtins) attrNames filter getAttr hasAttr head isAttrs isList listToAttrs;
in
rec {
  /*
   * Retrieves the property that uniquely identifies the target machine
   * (typically a string that specifies how to connect to it).
   *
   * Example:
   *   { properties.hostname = "test1.local";
   *     targetProperty = "hostname";
   *   }
   *   =>
   *   "test1.local"
   */
  getTargetProperty = {target, defaultTargetProperty}:
    if target ? targetProperty
    then getAttr target.targetProperty target.properties
    else getAttr defaultTargetProperty target.properties;

  /*
   * Turns a list with target machine configurations into a list of reference
   * specifications only containing the target and container name.
   *
   * Example:
   *   [ infrastructure.test1 ]
   *   =>
   *   [ { target = "test1"; container = "process"; } ] # container corresponds to type
   */
  normalizeDependencyTargetList = {targets, type, targetAliases, defaultTargetProperty}:
     map (target:
      let
        targetProperty = getTargetProperty {
          inherit target defaultTargetProperty;
        };
      in
      { target = getAttr targetProperty targetAliases;
        container = type;
      }
    ) targets;

  /*
   * Turns an attribute set with a verbose mapping to a target and container
   * to a list of reference specifications only containing the target and
   * container name.
   *
   * Example:
   *   { targets = [ {
   *       target = infrastructure.test1;
   *       container = "process";
   *     } ];
   *   }
   *   =>
   *   [ { target = "test1"; container = "process"; } ]
   */
  normalizeDependencyTargetAttrs = {targets, type, targetAliases, defaultTargetProperty}:
    map (target:
      let
        targetProperty = getTargetProperty {
          inherit (target) target;
          inherit defaultTargetProperty;
        };
      in
      {
        target = getAttr targetProperty targetAliases;
      } // (if target ? container then {
        inherit (target) container;
      } else {
        container = type;
      })
    ) targets.targets;

  /*
   * Turns mappings to containers on target machines into reference
   * specifications consisting of the target name and container name.
   */
  normalizeDependencyTargets = {targets, type, targetAliases, defaultTargetProperty}:
    if isList targets then normalizeDependencyTargetList {
      inherit targets type targetAliases defaultTargetProperty;
    }
    else if isAttrs targets then normalizeDependencyTargetAttrs {
      inherit targets type targetAliases defaultTargetProperty;
    }
    else throw "Unknown type for targets. It can only be a list or an attribute set";

  /**
   * Turns mappings to services as inter-dependencies (that are mapped to
   * containers on target machines) into inter-dependency reference
   * specifications containing of the service name, target name and container
   * name.
   *
   * Example:
   *   { mydep = services.mydep // {
   *       targets = {
   *         targets = { target = infrastructure.test1; container = "process"; };
   *       };
   *     };
   *   }
   *   =>
   *   { mydep = { service = "mydep"; target = "test1"; container = "process"; };
   *   }
   */
  normalizeInterDependencies = {dependencyParams, normalizedServices, targetAliases, defaultTargetProperty}:
    lib.mapAttrs (paramName: dependency:
      if dependency.type == "package"
      then throw "It is not allowed to have an inter-dependency on service: ${dependency.name} because it has a package type!" # Disallow inter-dependencies on package types
      else {
        service = dependency.name;
      } // (if dependency ? targets then {
        # If the dependency specifies targets, then normalize and use them
        targets = normalizeDependencyTargets {
          inherit (dependency) targets;
          inherit targetAliases defaultTargetProperty;
          type = normalizedServices."${dependency.name}".type;
        };
      } else {
        # If the dependency specifies no targets, adopt the targets from the service declaration
        targets = normalizedServices."${dependency.name}".targets;
      })
    ) dependencyParams;

  /*
   * Normalizes all inter-dependency references (specified by the connectsTo,
   * dependsOn and activatesAfter attributes) by changing them into reference
   * specifications.
   */
  normalizeAllInterDependencies = {service, targetAliases, normalizedServices, defaultTargetProperty}:
    lib.optionalAttrs (service ? connectsTo) {
      connectsTo = normalizeInterDependencies {
        dependencyParams = service.connectsTo;
        inherit targetAliases normalizedServices defaultTargetProperty;
      };
    }
    // lib.optionalAttrs (service ? dependsOn) {
      dependsOn = normalizeInterDependencies {
        dependencyParams = service.dependsOn;
        inherit targetAliases normalizedServices defaultTargetProperty;
      };
    }
    // lib.optionalAttrs (service ? activatesAfter) {
      activatesAfter = normalizeInterDependencies {
        dependencyParams = service.activatesAfter;
        inherit targetAliases normalizedServices defaultTargetProperty;
      };
    };

  /*
   * If a server defines a `providesContainer' string attribute, then it is
   * translated into a `providesContainers' attribute set that declares a
   * single container that exposes all the service's properties, except those
   * that have a reserved purpose.
   *
   * Example:
   *   { name = "myService";
   *     providesContainer = "myContainer";
   *     containerProperty = "test property";
   *   }
   *   =>
   *   { test = {
   *       containerProperty = "test property";
   *     };
   *   }
   */
  normalizeContainerProviderProperties = {service}:
    let
      reservedServiceAttrNames = [ "name" "pkg" "type" "deployState" "dependsOn" "connectsTo" "activatesAfter" "providesContainer" "providesContainers" "targets" ];
    in
    {
      "${service.providesContainer}" = removeAttrs service reservedServiceAttrNames;
    };

  /*
   * Translates a target reference of an inter-dependency to a format that is
   * consumable by Nix expressions that build and configure a service.
   *
   * It will expose all normalized properties of a target machine, with
   * the containers property removed, selectedContainer referring to the
   * selectedContainer and the container attribute exposing all the container
   * properties.
   *
   * If a target refers to a container provided by a service deployed to the
   * same target machine, then it will expose the properties provided by the
   * service, rather than the target machine in the infrastructure section.
   *
   * Example:
   *   { target = "test1"; container = "process"; }
   *   =>
   *   { properties.hostname = "test1";
   *     selectedContainer = "process";
   *     container = {
   *       stateDir = "/var";
   *     }
   *   }
   */
  generateInterDependencyTarget = {targetReference, normalizedServices, normalizedInfrastructure, serviceContainersPerTarget}:
    let
      target = getAttr targetReference.target normalizedInfrastructure;
    in
    removeAttrs target [ "containers" ] // {
      # TODO: add providedByService?
      selectedContainer = targetReference.container;
      container = if hasAttr targetReference.target serviceContainersPerTarget && hasAttr targetReference.container serviceContainersPerTarget."${targetReference.target}" # Can we simplify this?
        then normalizedServices."${serviceContainersPerTarget."${targetReference.target}"."${targetReference.container}"}".providesContainers."${targetReference.container}"
        else getAttr targetReference.container target.containers;
    };

  /*
   * Translates all target references of an inter-dependency parameter to a
   * format that is consumable by Nix expressions that build and configure a service.
   */
  generateInterDependencyTargets = {targetReferences, normalizedServices, normalizedInfrastructure, serviceContainersPerTarget}:
    map (targetReference:
      generateInterDependencyTarget {
        inherit targetReference normalizedServices normalizedInfrastructure serviceContainersPerTarget;
      }
    ) targetReferences;

  /*
   * Translates all inter-dependency parameters that propagate settings
   * (connectsTo and dependsOn) to an attribute set that can be consumed as
   * inter-dependency parameters by the Nix expressions that build and configure
   * an individual service.
   *
   * Example:
   *   { name = "myservice";
   *     type = "process";
   *     dependsOn = {
   *       mydependency = { service = "myotherservice"; container = "process"; target = "target1"; };
   *     };
   *   }
   *   =>
   *   { mydependency = rec {
   *       name = "myservice";
   *       type = "process";
   *       targets = [
   *         { properties = { ... }; selectedContainer = "process"; container = { ... }; }
   *       ];
   *       target = head target;
   *       # connectsTo has the same format but then for all transitive inter-dependencies
   *     };
   *   }
   */
  generateInterDependencyParams = {normalizedService, normalizedServices, normalizedInfrastructure, serviceContainersPerTarget}:
    let
      interDependencyParams = normalizedService.connectsTo or {} // normalizedService.dependsOn or {};
    in
    lib.mapAttrs (paramName: dependencyReference:
      let
        normalizedDependency = getAttr dependencyReference.service normalizedServices;

        transitiveInterDependencyParams = generateInterDependencyParams {
          normalizedService = normalizedDependency;
          inherit normalizedServices normalizedInfrastructure serviceContainersPerTarget;
        };
      in
      removeAttrs normalizedDependency [ "activatesAfter" "connectsTo" "dependsOn" ] // rec { # Do not directly expose the inter-dependencies, but have a connectsTo parameter that provides a consumable interface
        targets = generateInterDependencyTargets {
          targetReferences = dependencyReference.targets;
          inherit normalizedServices normalizedInfrastructure serviceContainersPerTarget;
        };
        target = head targets;
      } // lib.optionalAttrs (transitiveInterDependencyParams != {}) {
        connectsTo = transitiveInterDependencyParams; # Expose all transitive dependencies in the same format via the connectsTo property
      }
    ) interDependencyParams;

  /*
   * Generates a list of unique system targets from a list of references to targe machines
   *
   * Example:
   *   {
   *     normalizedInfrastructure = {
   *       test1.system = "x86_64-linux";
   *       test2.system = "x86_64-darwin";
   *       test3.system = "x86_64-linux";
   *     };
   *     targetReferences = [
   *       { target = "test1"; container = "process"; }
   *       { target = "test2"; container = "process"; }
   *       { target = "test3"; container = "process"; }
   *     ];
   *   }
   *   =>
   *   [ "x86_64-linux" "x86_64-darwin" ]
   */
  generateUniqueSystemsFromTargetReferences = {targetReferences, normalizedInfrastructure}:
    lib.unique (map (targetReference:
      let
        target = getAttr targetReference.target normalizedInfrastructure;
      in
      target.system
    ) targetReferences);

  /*
   * Lazily builds all package artifacts of a service for all potential system
   * architectures.
   *
   * To accomplish this it invokes the `pkg` function and optionally provides
   * consumable inter-dependency parameters so that the Nix expression for the
   * service can build and configure itself.
   *
   * Example result:
   *   { x86_64-linux = "/nix/store/....";
   *     x86_64-darwin = "/nix/store/....";
   *   }
   */
  generatePkgs = {normalizedService, normalizedServices, normalizedInfrastructure, architectureFun, extraParams, nixpkgs, serviceContainersPerTarget}:
    let
      systems = generateUniqueSystemsFromTargetReferences {
        targetReferences = normalizedService.targets;
        inherit normalizedInfrastructure;
      };

      interDependencyParams = generateInterDependencyParams {
        inherit normalizedService normalizedServices normalizedInfrastructure serviceContainersPerTarget;
      };
    in
    lib.genAttrs systems (system:
      let
        architecture = architectureFun ({
          pkgs = import nixpkgs { inherit system; };
          inherit system;
        } // extraParams);

        pkg = architecture.services."${normalizedService.name}".pkg;
      in
      if interDependencyParams == {}
      then pkg
      else pkg interDependencyParams
    );

  /*
   * Searches a list of target references for the first entry mapped to a
   * certain target machine.
   *
   * Example:
   *   findFirstContainerTarget { targetName = "test1"; targetReferences = [ { target = "test1"; container = "process"; } ]; }
   *   =>
   *  { target = "test1"; container = "process"; }
   */
  findFirstContainerTarget = {targetName, targetReferences}:
    lib.findFirst (target: target.target == targetName) null targetReferences;

  /*
   * Generates an attribute set of inter-dependencies for all service containers
   * that a service is mapped to.
   *
   * For example, if service `foo' is mapped to target machine: `test1' and
   * requires container: mycontainer, and the `bar' service that provides that
   * container is mapped to the same machine, then the result is:
   *
   * { bar = {
   *     service = "bar";
   *     targets = [ { target = "test1"; container = "process"; } ];
   *   };
   * }
   */
  generateServiceContainerInterDependencies = {service, services, serviceContainersPerTarget, targetAliases, defaultTargetProperty}:
    if service ? targets
    then
      let
        normalizedTargets = normalizeDependencyTargets {
          inherit (service) targets type;
          inherit targetAliases defaultTargetProperty;
        };
        serviceContainerTargets = filter (targetReference: hasAttr targetReference.target serviceContainersPerTarget && hasAttr targetReference.container serviceContainersPerTarget."${targetReference.target}") normalizedTargets;
      in
      listToAttrs (map (targetReference:
        let
          containerServiceName = serviceContainersPerTarget."${targetReference.target}"."${targetReference.container}";
          containerService = getAttr containerServiceName services;

          firstContainerTarget = findFirstContainerTarget {
            targetReferences = normalizeDependencyTargets {
              inherit (containerService) targets type;
              inherit targetAliases defaultTargetProperty;
            };
            targetName = targetReference.target;
          };

          activatesAfterConfig = {
            service = containerServiceName;
            targets = [ firstContainerTarget ];
          };
        in
        { name = containerServiceName;
          value = activatesAfterConfig;
        }
      ) serviceContainerTargets)
    else {};

  /*
   * For all services that are mapped to containers provided by other services,
   * append the latter category of services as `activatesAfter'
   * inter-dependencies to the former to ensure proper activation ordering.
   */
  appendContainerServiceInterDependencies = {service, services, serviceContainersPerTarget, targetAliases, defaultTargetProperty}:
    let
      containerServiceInterDependencies = generateServiceContainerInterDependencies {
        inherit service services serviceContainersPerTarget targetAliases defaultTargetProperty;
      };
    in
    if containerServiceInterDependencies == {} then {}
    else if service ? activatesAfter
      then {
        activatesAfter = service.activatesAfter // containerServiceInterDependencies;
      } else {
        activatesAfter = containerServiceInterDependencies;
      };

  /*
   * Turns mappings to containers on target machines into reference
   * specifications consisting of the target name, container name, and
   * (optionally) a containerProviderService, if applicable.
   */
  normalizeServiceTargets = {service, defaultTargetProperty, targetAliases, serviceContainersPerTarget}:
    let
      normalizedDependencyTargets = normalizeDependencyTargets {
        inherit (service) targets type;
        inherit targetAliases defaultTargetProperty;
      };
    in
    map (normalizedDependencyTarget:
      normalizedDependencyTarget // lib.optionalAttrs (hasAttr normalizedDependencyTarget.target serviceContainersPerTarget && hasAttr normalizedDependencyTarget.container serviceContainersPerTarget."${normalizedDependencyTarget.target}") {
        containerProvidedByService = serviceContainersPerTarget."${normalizedDependencyTarget.target}"."${normalizedDependencyTarget.container}";
      }
    ) normalizedDependencyTargets;

  /*
   * Normalizes the configuration of a service by:
   *
   * - Providing default properties for certain unspecified configuration options
   * - Translating a single container provider to an attribute set of container providers
   * - Translating all target references and inter-dependencies to reference specifications
   * - Appending services that are mapped to services containers as inter-dependencies
   * - Evaluating all package builds for every unique system target
   */
  normalizeService = {service, services, defaultDeployState, defaultTargetProperty, targetAliases, normalizedServices, normalizedInfrastructure, architectureFun, extraParams, nixpkgs, serviceContainersPerTarget}:
    let
      normalizedServiceTargets = normalizeDependencyTargets {
        inherit (service) targets type;
        inherit targetAliases defaultTargetProperty;
      };

      normalizedService =
        # Provide the default properties
        {
          deployState = defaultDeployState;
        }
        # Override with the service properties
        // service
        # Derive providesContainers attribute set from providesContainer
        // lib.optionalAttrs (service ? providesContainer) {
          providesContainers = normalizeContainerProviderProperties {
            inherit service;
          };
        }
        // lib.optionalAttrs (service ? targets) {
          # Normalize targets to references
          targets = normalizeServiceTargets {
            inherit service defaultTargetProperty targetAliases serviceContainersPerTarget;
          };
          # Generate package builds per unique system architecture
          pkgs = generatePkgs {
            inherit normalizedService normalizedServices normalizedInfrastructure architectureFun extraParams nixpkgs serviceContainersPerTarget;
          };
        }
        # Normalize all inter-dependencies by turning them into references
        // normalizeAllInterDependencies {
          inherit service targetAliases normalizedServices defaultTargetProperty;
        }
        # Append all services that are container targets as activatesAfter inter-dependencies
        // appendContainerServiceInterDependencies {
          inherit service services serviceContainersPerTarget targetAliases defaultTargetProperty;
        };
    in
    normalizedService;

  /*
   * Checks whether the name of every service matches the attribute name.
   * If an inconsistency is detected, then calling the service throws an exception.
   */
  checkServiceNames = {services}:
    lib.mapAttrs (name: service:
      if !(service ? name) then throw "Mandatory name attribute for service: ${name} is missing!"
      else if name != service.name then throw "The name attribute for service: ${name} should be: ${name}, instead it is: ${service.name}"
      else service
    ) services;

  /*
   * Generates an attribute set that for each target, a mapping from the
   * container to the service provides that provides the container.
   *
   * This attribute set can be used to identify whether a container is provided
   * by a service or is already predeployed to a target machine and if so, which service
   * provides it so that we can propagate the container properties.
   *
   * Example:
   *   { test1.mysql-database = "mysqlPrimary";
   *     test2.apache-webapplication = "simpleWebappApache";
   *   }
   */
  generateServiceContainersPerTarget = {services, targetAliases, defaultTargetProperty}:
    let
      targetContainerToServiceMappingsPerService = map (serviceName:
        let
          service = getAttr serviceName services;

          normalizedTargets = normalizeDependencyTargets {
            inherit (service) targets type;
            inherit targetAliases defaultTargetProperty;
          };

          uniqueTargetNames = lib.unique (map (targetReference: targetReference.target) normalizedTargets);

          providesContainers = if service ? providesContainers
            then service.providesContainers
            else if service ? providesContainer
              then normalizeContainerProviderProperties {
                inherit service;
              }
              else {};
        in
        lib.genAttrs uniqueTargetNames (targetName:
          lib.mapAttrs (containerName: properties: service.name) providesContainers
        )
      ) (attrNames services);
    in
    lib.foldr (targetContainerToServiceMappingsForAService: serviceContainersPerTarget:
      lib.recursiveUpdate serviceContainersPerTarget targetContainerToServiceMappingsForAService
    ) {} targetContainerToServiceMappingsPerService;

  /*
   * Filters out only the services that have targets.
   *
   * Example:
   *   {
   *     foo = { name = "foo"; targets = [ infrastructure1 ]; };
   *     bar = { name = "bar"; };
   *   }
   *   =>
   *   {
   *     foo = { name = "foo"; targets = [ infrastructure1 ]; };
   *   }
   */
  filterServicesWithTargets = {services}:
    lib.filterAttrs (serviceName: service: service ? targets) services;

  /**
   * Normalizes the services section in the architecture model by:
   * - Checking whether the services' names match the attribute names
   * - Substituting relevant unspecified properties with default values
   * - Turning all dependency and taget properties into reference specifications
   * - Augmenting inter-dependencies to services that are mapped to a container provided by a service
   * - Building all packages for every system architecture the service is distributed to by passing the appropriate inter-dependency function parameters
   */
  normalizeServices = {services, defaultDeployState, defaultTargetProperty, normalizedInfrastructure, architectureFun, extraParams, targetAliases, nixpkgs}:
    let
      servicesWithTargets = filterServicesWithTargets {
        inherit services;
      };

      serviceContainersPerTarget = generateServiceContainersPerTarget {
        services = servicesWithTargets;
        inherit targetAliases defaultTargetProperty;
      };

      checkedServices = checkServiceNames {
        services = servicesWithTargets;
      };

      normalizedServices = lib.mapAttrs (serviceName: service:
        normalizeService {
          inherit service services defaultDeployState defaultTargetProperty targetAliases normalizedServices normalizedInfrastructure architectureFun extraParams nixpkgs serviceContainersPerTarget;
        }
      ) servicesWithTargets;
    in
    normalizedServices;
}
