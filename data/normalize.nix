{lib}:

let
  inherit (builtins) isList isAttrs head getAttr filter attrNames concatLists listToAttrs;

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

  normalizeTarget = {target, defaultClientInterface, defaultTargetProperty}:
    { clientInterface = defaultClientInterface;
      targetProperty = defaultTargetProperty;
      numOfCores = 1;
      system = builtins.currentSystem;
    } // target;

  normalizeInfrastructure = {architecture, defaultClientInterface, defaultTargetProperty}:
    architecture // {
      infrastructure = lib.mapAttrs (name: target: normalizeTarget {
        inherit target defaultClientInterface defaultTargetProperty;
      }) architecture.infrastructure;
    };

  selectContainers = {targets, type}:
    let
      generateTargetWithoutContainers = target:
        removeAttrs target [ "containers" ];
    in
    if isList targets then map (target:
      let
        selectedContainer = type;
      in
      { # When targets are a list, do automapping of type to container
        container = target.containers."${selectedContainer}";
        inherit selectedContainer;
      } // (generateTargetWithoutContainers target)) targets
    else if isAttrs targets then map (mapping:
      let
        selectedContainer = mapping.container or type; # If target specifies a container, map to that container. If target does not specify a container, do an automap of the type to the container
      in
      { # When targets is an attribute set, use the more advanced notation
        container = mapping.target.containers."${selectedContainer}";
        inherit selectedContainer;
      } // (generateTargetWithoutContainers mapping.target)) targets.targets
    else throw "targets has the wrong type!";

  selectContainersInServiceTargets = {architecture}:
    architecture // {
      services = lib.mapAttrs (name: service: service // {
        targets = selectContainers {
          inherit (service) targets type;
        };
      }) architecture.services;
    };

  normalizeTargets = {targets, defaultClientInterface, defaultTargetProperty}:
    map (target: normalizeTarget {
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

  augmentTargetsToInterDependencies = {architectureWithTargets}:
    let
      appendTargetsToDependencies = dependencies:
        lib.mapAttrs (name: dependency:
          rec {
            targets = if dependency ? targets
              then normalizeTargets (selectContainers dependency.targets)
              else architectureWithTargets.services."${dependency.name}".targets;
            target = head targets;
          } // (removeAttrs dependency [ "targets" "target" ])
        ) dependencies;
    in
    architectureWithTargets // {
      services = lib.mapAttrs (name: service: service // {
        dependsOn = if service ? dependsOn then appendTargetsToDependencies service.dependsOn else {};
        connectsTo = if service ? connectsTo then appendTargetsToDependencies service.connectsTo else {};
      }) architectureWithTargets.services;
    };

  getTargetProperty = target:
    getAttr target.targetProperty target.properties;

  evaluatePkgsPerSystem = {architecture, architectureFun, nixpkgs}:
    architecture // {
      services = lib.mapAttrs (name: service:
        let
          systems = lib.unique (map (target: target.system) (service.targets));
          interDependencies = (service.connectsTo or {}) // (service.dependsOn or {});

          buildFun = system:
            (architectureFun { inherit system; pkgs = import nixpkgs { inherit system; }; }).services."${name}".pkg;
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

  generateNormalizedDeploymentArchitecture = {architectureFun, nixpkgs, defaultClientInterface, defaultTargetProperty, defaultDeployState}:
    let
      architectureBasis = architectureFun {
        pkgs = import nixpkgs {}; system = null; # TODO: Use default pkgs or use 'lib' parameter (the latter is probably better)
      };

      checkedArchitecture = checkServiceNames {
        architecture = architectureBasis;
      };

      architectureWithDeployState = normalizeDeployState {
        architecture = checkedArchitecture;
        inherit defaultDeployState;
      };

      architectureWithNormalizedInfrastructure = normalizeInfrastructure {
        architecture = architectureWithDeployState;
        inherit defaultClientInterface defaultTargetProperty;
      };

      architectureWithSelectedContainers = selectContainersInServiceTargets {
        architecture = architectureWithNormalizedInfrastructure;
      };

      architectureWithNormalizedServiceTargets = normalizeServiceTargets {
        architecture = architectureWithSelectedContainers;
        inherit defaultClientInterface defaultTargetProperty;
      };

      architectureWithInterDependencyTargets = augmentTargetsToInterDependencies {
        architectureWithTargets = architectureWithNormalizedServiceTargets;
      };

      architectureWithPkgsPerSystem = evaluatePkgsPerSystem {
        architecture = architectureWithInterDependencyTargets;
        inherit architectureFun nixpkgs;
      };

      architectureWithPkgsPerTarget = generatePkgsPerTarget {
        architecture = architectureWithPkgsPerSystem;
      };
    in
    architectureWithPkgsPerTarget;
in
generateNormalizedDeploymentArchitecture
