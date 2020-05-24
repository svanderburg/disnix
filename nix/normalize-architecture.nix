{lib}:

let
  inherit (builtins) isList isAttrs head getAttr filter attrNames concatLists listToAttrs elem hasAttr;

  normalizeInfrastructure = import ./normalize-infrastructure.nix {
    inherit lib;
  };

  checkServiceNames = {architecture}:
    architecture // {
      services = lib.mapAttrs (name: service:
        if !(service ? name) then throw "Mandatory name attribute for service: ${name} is missing!"
        else if name != service.name then "The name attribute for service: ${name} should be: ${name}, instead it is: ${service.name}"
        else service
      ) architecture.services;
    };

  normalizeDeployState = {architecture, defaultDeployState}:
    architecture // {
      services = lib.mapAttrs (name: service:
        {
          deployState = defaultDeployState;
        } // service
      ) architecture.services;
    };

  normalizeInfrastructureAttribute = {architecture, defaultClientInterface, defaultTargetProperty}:
    architecture // {
      infrastructure = normalizeInfrastructure.normalizeInfrastructure {
        inherit (architecture) infrastructure;
        inherit defaultClientInterface defaultTargetProperty;
      };
    };

  generateArchitectureWithNormalizedServiceContainerProperties = {architecture}:
    let
      normalizeServiceContainerProperties = {service}:
        if service ? providesContainer then {
          providesContainers = {
            "${service.providesContainer}" = removeAttrs service [ "name" "pkg" "type" "deployState" "dependsOn" "connectsTo" "activatesAfter" "providesContainer" "providesContainers" "targets" ];
          } // service.providesContainers or {};
        } // service else service;
    in
    architecture // {
      services = lib.mapAttrs (name: service:
        normalizeServiceContainerProperties {
          inherit service;
        }
      ) architecture.services;
    };

  getTargetProperty = target:
    getAttr target.targetProperty target.properties;

  generateServiceContainersPerTarget = {architecture, defaultTargetProperty}:
    let
      convertMappingsListToTargetsList = mappings:
        map (mapping: mapping.target) mappings;

      findServicesPerTarget = {target}:
        lib.filterAttrs (name: service:
          if service ? targets then
            if isList service.targets then elem target service.targets
            else if isAttrs service.targets then elem target (convertMappingsListToTargetsList service.targets.targets)
            else throw "Unknown targets type for service: ${name}!"
          else false
        ) architecture.services;

      servicesPerTarget = listToAttrs (map (targetName:
        let
          target = getAttr targetName architecture.infrastructure;
          targetProperty = if target ? targetProperty then target.targetProperty else defaultTargetProperty;
        in
        { name = getAttr targetProperty target.properties;
          value = findServicesPerTarget {
            inherit target;
          };
        }
      ) (attrNames architecture.infrastructure));
    in
    lib.mapAttrs (targetName: services:
      listToAttrs (lib.flatten (map (serviceName:
        let
          service = getAttr serviceName services;
        in
        if service ? providesContainers then
          map (containerName:
            { name = containerName;
              value = {
                providedByService = serviceName;
                properties = getAttr containerName service.providesContainers;
              };
            }
          ) (attrNames service.providesContainers)
        else []
      ) (attrNames services)))
    ) servicesPerTarget;

  selectContainers = {targets, serviceContainersPerTarget, type, defaultTargetProperty}:
    let
      generateTargetWithoutContainers = target:
        removeAttrs target [ "containers" ];
    in
    if isList targets then map (target:
      let
        selectedContainer = type;
        targetPropertyAttr = if target ? targetProperty then target.targetProperty else defaultTargetProperty;
        targetProperty = target.properties."${targetPropertyAttr}";
      in
      { # When targets are a list, do automapping of type to container
        container = serviceContainersPerTarget."${targetProperty}"."${selectedContainer}".properties or target.containers."${selectedContainer}";
        inherit selectedContainer;
      } // lib.optionalAttrs (hasAttr targetProperty serviceContainersPerTarget && hasAttr selectedContainer (serviceContainersPerTarget."${targetProperty}")) {
        providedByService = serviceContainersPerTarget."${targetProperty}"."${selectedContainer}".providedByService;
      } // (generateTargetWithoutContainers target)) targets
    else if isAttrs targets then map (mapping:
      let
        selectedContainer = mapping.container or type; # If target specifies a container, map to that container. If target does not specify a container, do an automap of the type to the container
        targetPropertyAttr = if mapping.target ? targetProperty then mapping.target.targetProperty else defaultTargetProperty;
        targetProperty = mapping.target.properties."${targetPropertyAttr}";
      in
      { # When targets is an attribute set, use the more advanced notation
        container = serviceContainersPerTarget."${targetProperty}"."${selectedContainer}".properties or mapping.target.containers."${selectedContainer}";
        inherit selectedContainer;
      } // lib.optionalAttrs (hasAttr targetProperty serviceContainersPerTarget && hasAttr selectedContainer (serviceContainersPerTarget."${targetProperty}")) {
        providedByService = serviceContainersPerTarget."${targetProperty}"."${selectedContainer}".providedByService;
      } // (generateTargetWithoutContainers mapping.target)) targets.targets
    else throw "targets has the wrong type!";

  selectContainersInServiceTargets = {architecture, serviceContainersPerTarget, defaultTargetProperty}:
    architecture // {
      services = lib.mapAttrs (name: service: service // {
        targets = selectContainers {
          inherit (service) targets type;
          inherit serviceContainersPerTarget defaultTargetProperty;
        };
      }) architecture.services;
    };

  normalizeTargets = {targets, defaultClientInterface, defaultTargetProperty}:
    map (target: normalizeInfrastructure.normalizeTarget {
      inherit target defaultClientInterface defaultTargetProperty;
    }) targets;

  normalizeServiceTargets = {architecture, defaultClientInterface, defaultTargetProperty}:
    architecture // {
      services = lib.mapAttrs (name: service: service // {
        targets = normalizeTargets {
          inherit defaultClientInterface defaultTargetProperty;
          inherit (service) targets;
        };
      }) architecture.services;
    };

  augmentTargetsToInterDependencies = {architectureWithTargets, serviceContainersPerTarget, defaultClientInterface, defaultTargetProperty}:
    let
      appendTargetsToDependencies = dependencies:
        lib.mapAttrs (name: dependency:
          rec {
            targets = if dependency ? targets
              then normalizeTargets {
                targets = selectContainers {
                  inherit (dependency) targets type;
                  inherit serviceContainersPerTarget defaultTargetProperty;
                };
                inherit defaultClientInterface defaultTargetProperty;
              }
              else architectureWithTargets.services."${dependency.name}".targets;

            target = head targets;
          } // (removeAttrs dependency [ "targets" "target" ])
        ) dependencies;
    in
    architectureWithTargets // {
      services = lib.mapAttrs (name: service: service // {
        dependsOn = if service ? dependsOn then appendTargetsToDependencies service.dependsOn else {};
        connectsTo = if service ? connectsTo then appendTargetsToDependencies service.connectsTo else {};
        activatesAfter = if service ? activatesAfter then appendTargetsToDependencies service.activatesAfter else {};
      }) architectureWithTargets.services;
    };

  evaluatePkgsPerSystem = {architecture, architectureFun, nixpkgs, extraParams}:
    architecture // {
      services = lib.mapAttrs (name: service:
        let
          systems = lib.unique (map (target: target.system) (service.targets));
          interDependencies = (service.connectsTo or {}) // (service.dependsOn or {});

          buildFun = system:
            (architectureFun ({
              inherit system;
              pkgs = import nixpkgs { inherit system; };
            }) // extraParams).services."${name}".pkg;
        in
        service // {
          _systemsPerTarget = map (target:
            { target = getTargetProperty target;
              inherit (target) system;
            }
          ) service.targets;

          _pkgsPerSystem = lib.genAttrs systems (system:
            if interDependencies == {}
            then buildFun system # If no inter-dependencies are provided, just invoke the function as it is
            else (buildFun system) interDependencies # If inter-dependencies are provided, pass them as a parameter to the build function
          );
        }) architecture.services;
    };

  generatePkgsPerTarget = {architecture}:
    let
      generatePkgsPerTargetGroupedByService = {architecture}:

        lib.mapAttrs (name: target:
          let
            targetProperty = getTargetProperty target;
          in
          map (serviceName:
            let
              service = getAttr serviceName architecture.services;
              mappingsToTarget = filter (mapping: mapping.target == targetProperty) service._systemsPerTarget;
            in
            lib.unique (map (mapping: service._pkgsPerSystem."${mapping.system}") mappingsToTarget)
          ) (attrNames architecture.services)
        ) architecture.infrastructure;

      pkgsFromServices = lib.mapAttrs (targetName: pkgsPerService:
        concatLists pkgsPerService
      ) (generatePkgsPerTargetGroupedByService { inherit architecture; });

      allTargetNames = attrNames (pkgsFromServices // (architecture.targetPackages or {}));
    in
    architecture // {
      # Merge existing pkgs attribute with pkgs that need to be built for the services
      targetPackages = listToAttrs (map (targetName: {
        name = targetName;
        value = (pkgsFromServices.${targetName} or []) ++ (architecture.targetPackages.${targetName} or []);
      }) allTargetNames);
    };

  filterServiceContainerTargets = {targets}:
    filter (target: target ? providedByService) targets;

  convertServiceContainerTargetsToInterDependencies = {services, targets}:
    let
      containerMappingPerTarget = (map (target:
        let
          containerService = getAttr target.providedByService services;
          serviceTargetProperty = getTargetProperty target;
        in
        containerService // {
          targets = filter (target:
            let
              containerTargetProperty = getTargetProperty target;
            in
            serviceTargetProperty == containerTargetProperty
          ) containerService.targets;
        }
      ) targets);
    in
    lib.foldr (service: dependencies:
      dependencies // {
        "${service.name}" = (if hasAttr service.name dependencies then service // {
          targets = dependencies."${service.name}".targets or [] ++ service.targets or [];
        } else service);
      }
    ) {} containerMappingPerTarget;

  augmentServiceContainerDependenciesToService = {services, service}:
    let
      serviceContainerTargets = filterServiceContainerTargets {
        inherit (service) targets;
      };

      extraInterDependencies = convertServiceContainerTargetsToInterDependencies {
        inherit services;
        targets = serviceContainerTargets;
      };
    in
    service // lib.optionalAttrs (extraInterDependencies != {}) {
      activatesAfter = service.activatesAfter or {} // extraInterDependencies;
    };

  augmentServiceContainerDependencies = {architecture}:
    architecture // {
      services = lib.mapAttrs (serviceName: service:
        augmentServiceContainerDependenciesToService {
          inherit (architecture) services;
          inherit service;
        }
      ) architecture.services;
    };

  generateNormalizedDeploymentArchitecture = {architectureFun, nixpkgs, defaultClientInterface, defaultTargetProperty, defaultDeployState, extraParams}:
    let
      architectureBasis = architectureFun ({
        pkgs = import nixpkgs {}; system = null; # TODO: Use default pkgs or use 'lib' parameter (the latter is probably better)
      } // extraParams);

      checkedArchitecture = checkServiceNames {
        architecture = architectureBasis;
      };

      architectureWithDeployState = normalizeDeployState {
        architecture = checkedArchitecture;
        inherit defaultDeployState;
      };

      architectureWithNormalizedServiceContainerProperties = generateArchitectureWithNormalizedServiceContainerProperties {
        architecture = architectureWithDeployState;
      };

      serviceContainersPerTarget = generateServiceContainersPerTarget {
        architecture = architectureWithNormalizedServiceContainerProperties;
        inherit defaultTargetProperty;
      };

      architectureWithNormalizedInfrastructure = normalizeInfrastructureAttribute {
        architecture = architectureWithNormalizedServiceContainerProperties;
        inherit defaultClientInterface defaultTargetProperty;
      };

      architectureWithSelectedContainers = selectContainersInServiceTargets {
        architecture = architectureWithNormalizedInfrastructure;
        inherit serviceContainersPerTarget defaultTargetProperty;
      };

      architectureWithNormalizedServiceTargets = normalizeServiceTargets {
        architecture = architectureWithSelectedContainers;
        inherit defaultClientInterface defaultTargetProperty;
      };

      architectureWithServiceContainerDependencies = augmentServiceContainerDependencies {
        architecture = architectureWithNormalizedServiceTargets;
      };

      architectureWithInterDependencyTargets = augmentTargetsToInterDependencies {
        architectureWithTargets = architectureWithServiceContainerDependencies;
        inherit serviceContainersPerTarget defaultClientInterface defaultTargetProperty;
      };

      architectureWithPkgsPerSystem = evaluatePkgsPerSystem {
        architecture = architectureWithInterDependencyTargets;
        inherit architectureFun nixpkgs extraParams;
      };

      architectureWithPkgsPerTarget = generatePkgsPerTarget {
        architecture = architectureWithPkgsPerSystem;
      };
    in
    architectureWithPkgsPerTarget;
in
generateNormalizedDeploymentArchitecture
