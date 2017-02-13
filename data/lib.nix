{nixpkgs, pkgs}:

let
  inherit (builtins) attrNames getAttr listToAttrs head tail unsafeDiscardOutputDependency hashString filter elem isList isAttrs;
in
rec {
  /**
   * Checks whether the convention for the name attributes is followed properly.
   *
   * Parameters:
   * services: Services attribute set
   *
   * Returns:
   * The services model or an exception if the naming convention is violated
   */
  checkServiceNames = services:
    listToAttrs (map (serviceName:
      let service = getAttr serviceName services;
      in
      { name = serviceName;
        value =  if !(service ? name) then throw "Mandatory name attribute for service: ${serviceName} missing"
          else if service.name != serviceName then throw "The name attribute for service: ${serviceName} should be: ${serviceName}, instead it is: ${service.name}"
          else service;
      }
    ) (attrNames services))
  ;

  /*
   * Determines the right pkgs collection from the system identifier.
   *
   * Parameters:
   * system: System identifier
   *
   * Returns:
   * Packages collection for the given system identifier
   */
  
  selectPkgs = system:
    import nixpkgs { inherit system; };
  
  /*
   * Iterates over each service in the distribution attributeset, adds the corresponding service
   * declaration in the attributeset and augments the targets of every inter-dependency
   * in the dependsOn attributeset
   * 
   * Parameters:
   * distribution: Distribution attributeset
   * services: Initial services attributeset
   *
   * Returns:
   * Attributeset with service declarations augmented with targets in the dependsOn attribute
   */
   
  augmentTargetsInDependsOn = distribution: services:
    listToAttrs (map (serviceName:
      let service = getAttr serviceName services;
      in
      { name = serviceName;
        value = service // {
          dependsOn = if service ? dependsOn && service.dependsOn != {} then
            listToAttrs (map (argName:
              let
                dependencyName = (getAttr argName (service.dependsOn)).name;
                dependency = getAttr dependencyName services;
                targets = getAttr dependencyName distribution;
                
                targetParameters = if isList targets then
                  map (target:
                    { inherit (target) properties;
                      container = getAttr dependency.type target.containers;
                    }
                  ) targets
                else if isAttrs targets then
                  map (mapping:
                    { inherit (mapping.target) properties;
                      container = if mapping ? container
                        then getAttr mapping.container mapping.target.containers
                        else getAttr dependency.type mapping.target.containers;
                    }
                  ) targets.targets
                else throw "One of the targets in the distribution model is neither a list nor an attribute set!";
              in
              { name = argName;
                value = dependency // {
                  targets = targetParameters;
                  target = head targetParameters;
                };
              }
            ) (attrNames (service.dependsOn)))
          else {};
        };
      }
    ) (attrNames distribution))
  ;
  
  /*
   * Fetches the key value that is used to refer to a target machine.
   * If a target defines a 'targetProperty' then the corresponding attribute
   * is used. If no targetProperty is provided by the target, then the global
   * targetProperty is used.
   *
   * Parameters:
   * targetProperty: Attribute from the infrastructure model that is used to connect to the Disnix interface
   * target: An attributeset containing properties of a target machine
   *
   * Returns
   * A string containing the key value
   */
  getTargetProperty = targetProperty: target:
    if target ? targetProperty then getAttr (target.targetProperty) target.properties
    else getAttr targetProperty target.properties
  ;
  
  /*
   * Selects the pkg property of a specific service with all the required
   * parameters to evaluate it.
   *
   * Parameters:
   * servicesFun: Function that returns a services attributeset (defined in the services.nix file)
   * serviceName: Name of the service to select
   * distribution: Distribution attribute set
   * invDistribution: Inverse distribution attribute set
   * system: System identifier
   *
   * Returns:
   * The pkg property (a nested function taking intra- and inter dependency
   * parameters) that can be evaluated to build it
   */
  selectPkgProperty = servicesFun: serviceName: distribution: invDistribution: system:
    (getAttr serviceName (servicesFun { inherit distribution invDistribution; inherit system; pkgs = selectPkgs system; })).pkg;
  
  /*
   * Evaluates the service package and produces a store or derivation path from
   * it.
   *
   * Parameters:
   * service: Service to evaluate
   * serviceProperty: Defines which property we need of a derivation (either "outPath" or "drvPath")
   * pkg: Composed package property that needs to be evaluated
   *
   * Returns:
   * A store path to the build output or derivation file
   */
  evaluateService = service: serviceProperty: pkg:
    if serviceProperty == "outPath" then
      if service.dependsOn == {} then pkg.outPath
      else (pkg (service.dependsOn)).outPath
    else
      if service.dependsOn == {} then unsafeDiscardOutputDependency (pkg.drvPath)
      else unsafeDiscardOutputDependency ((pkg (service.dependsOn)).drvPath)
    ;
  
  /*
   * Iterates over all services in the distribution attributeset. For every
   * service, the attribute 'distribution' is added to the service declaration
   * which is a list that maps derivations to target machines.
   *
   * The distribution list is created by iterating over the targets in the distribution
   * attributeset of a specific service. For every target the services function is used
   * to retrieve the 'pkg' attribute. To the service function the 'system' attribute from
   * the infrastructure model is passed, so that it will be built for the right platform.
   * If the system attribute is omitted the system attribute of the coordinator system is used.
   * To the 'pkg' attribute (which is a function) the dependsOn attributeset is passed as argument.
   *
   * By requesting the outPath property of the derivation the service is built or by
   * requesting the drvPath property a store derivation file is created.
   *
   * Parameters:
   * distribution: Distribution attribute set
   * invDistribution: Inverse distribution attribute set
   * services: Services attribute set, augmented with targets in dependsOn
   * servicesFun: Function that returns a services attributeset (defined in the services.nix file)
   * serviceProperty: Defines which property we need of a derivation (either "outPath" or "drvPath")
   * targetProperty: Attribute from the infrastructure model that is used to connect to the Disnix interface
   *
   * Returns:
   * Service attributeset augmented with a distribution mapping property 
   */
  
  evaluatePkgFunctions = distribution: invDistribution: services: servicesFun: serviceProperty: targetProperty:
    listToAttrs (map (serviceName:
      let
        targets = getAttr serviceName distribution;
        service = getAttr serviceName services;
      in
      { name = serviceName;
        value = service // {
          distribution =
            if isList targets then
              map (target:
                let
                  system = if target ? system then target.system else builtins.currentSystem;
                  pkg = selectPkgProperty servicesFun serviceName distribution invDistribution system;
                in
                { service = evaluateService service serviceProperty pkg;
                  target = getTargetProperty targetProperty target;
                  container = service.type;
                }
              ) targets
            else if isAttrs targets then
              map (mapping:
                let
                  system = if mapping.target ? system then mapping.target.system else builtins.currentSystem;
                  pkg = selectPkgProperty servicesFun serviceName distribution invDistribution system;
                in
                { service = evaluateService service serviceProperty pkg;
                  target = getTargetProperty targetProperty mapping.target;
                  container = if mapping ? container then mapping.container else service.type;
                }
              ) targets.targets
            else throw "A service in a distribution model should either refer to a list or an attribute set!";
        };
      } 
    ) (attrNames distribution))
  ;

  /*
   * Composes a unique hash key for a service based on its properties that affect
   * deployment.
   *
   * Parameters:
   * services: The list of all services to deploy
   * service: The service for which a hash code must be generated
   * distributionItem: A mapping of build instance of this service to a machine
   *
   * Returns:
   * A SHA256 hash code
   */
  generateServiceHashKey = services: service: distributionItem:
    hashString "sha256" (builtins.toXML {
      inherit (distributionItem) service;
      inherit (service) name type;
      dependsOn = generateDependencyMapping (attrNames (service.dependsOn)) (service.dependsOn) services;
    })
  ;

  /*
   * Maps a list of inter-dependency declarations to a list of attribute sets
   * containing the Nix store path of the service and the targetProperty.
   *
   * Parameters:
   * argNames: argument names of the dependsOn attributeset
   * dependsOn: dependsOn attributeset from a service declaration
   * services: Services attributeset, augmented with distributions
   *
   * Returns:
   * List of derivation, target pairs
   */
   
  generateDependencyMapping = argNames: dependsOn: services:
    let
      dependency = getAttr (head argNames) dependsOn;
      serviceName = dependency.name;
      service = if dependency.type == "package"
        then throw "It is not allowed to refer to package: ${serviceName} as an inter-dependency!"
        else getAttr serviceName services;
      dependencyMappingItems = map (distributionItem:
        { _key = generateServiceHashKey services service distributionItem;
          inherit (distributionItem) target container;
        }
      ) (service.distribution);
    in
    if argNames == [] then [] else dependencyMappingItems ++ generateDependencyMapping (tail argNames) dependsOn services
  ;

  /*
   * For every distribution item of every service, a mapping attribute is created
   * which is a list of attributesets containing a service, target, type, targetProperty and dependsOn.
   *
   * Parameters:
   * serviceNames: List of names of services in the service attributeset
   * services: Services attributeset
   * targetProperty: Attribute from the infrastructure model that is used to connect to the Disnix interface
   *
   * Returns:
   * List of mappings
   */

  generateServiceActivationMapping = serviceNames: services: targetProperty:
    let
      service = getAttr (head serviceNames) services;
      dependsOn = generateDependencyMapping (attrNames (service.dependsOn)) (service.dependsOn) services;
      mappingItem = map (distributionItem:
        { inherit (distributionItem) service target container;
          inherit (service) name type;
          inherit dependsOn;
          _key = generateServiceHashKey services service distributionItem;
        }
      ) (service.distribution);
    in
    if serviceNames == [] then [] else mappingItem ++ (generateServiceActivationMapping (tail serviceNames) services targetProperty)
  ;
 
  /**
   * For a selected subset of distribution items of every service, a snapshot
   * mapping is created which is a list of attributesets containing a component
   * name, container name, and target.
   *
   * Parameters:
   * serviceNames: List of names of services in the service attributeset
   * services: Services attributeset
   * deployState: Indicates whether to globally deploy state
   *
   * Returns:
   * List of mappings
   */
   
  generateSnapshotsMapping = serviceNames: services: deployState:
    let
      service = getAttr (head serviceNames) services;
      
      distribution = if deployState || (service ? deployState && service.deployState) then service.distribution else []; # Only do state deployment with services that are annotated as such
      
      mappingItem = map (distributionItem:
        { component = builtins.substring 33 (builtins.stringLength (distributionItem.service)) (builtins.baseNameOf (distributionItem.service));
          inherit (service) type;
          inherit (distributionItem) target container service;
        }
      ) distribution;
    in
    if serviceNames == [] then [] else mappingItem ++ (generateSnapshotsMapping (tail serviceNames) services deployState)
  ;
  
  /*
   * Iterates over a service activation mapping list and filters out all the
   * services that are distributed to a specific target.
   *
   * Parameters:
   * serviceActivationMapping: List of activation mappings
   * targetName: Name of the target in the infrastructure model to filter on 
   * infrastructure: Infrastructure attributeset
   * targetProperty: Attribute from the infrastructure model that is used to connect to the Disnix interface
   *
   * Returns:
   * List of services that are distributed to the given target name
   */
   
  queryServicesByTargetName = serviceActivationMapping: targetName: infrastructure: targetProperty:
    if serviceActivationMapping == [] then []
    else
      let
        target = getAttr targetName infrastructure;
      in
      if (head serviceActivationMapping).target == getTargetProperty targetProperty target
      then [ (head serviceActivationMapping).service ] ++ (queryServicesByTargetName (tail serviceActivationMapping) targetName infrastructure targetProperty)
      else queryServicesByTargetName (tail serviceActivationMapping) targetName infrastructure targetProperty
  ;

  /*
   * Iterates over a service activation mapping list and filters out all the
   * services and types that are distributed to a specific target.
   *
   * Parameters:
   * serviceActivationMapping: List of activation mappings
   * targetName: Name of the target in the infrastructure model to filter on 
   * infrastructure: Infrastructure attributeset
   * targetProperty: Attribute from the infrastructure model that is used to connect to the Disnix interface
   * 
   * Returns:
   * List of services and types that are distributed to the given target name
   */
  
  generateProfileManifest = serviceActivationMapping: targetName: infrastructure: targetProperty:
    if serviceActivationMapping == [] then []
    else
      let
        target = getAttr targetName infrastructure;
        mapping = head serviceActivationMapping;
        stateful = if mapping ? deployState && mapping.deployState then "true" else "false";
        dependsOn = "[${toString (map (dependency: "{ target = \"${dependency.target}\"; container = \"${dependency.container}\"; _key = \"${dependency._key}\" }") (mapping.dependsOn))}]";
      in
      if mapping.target == getTargetProperty targetProperty target && mapping.type != "package"
      then [ mapping.name mapping.service mapping.container mapping.type mapping._key stateful dependsOn ] ++ (generateProfileManifest (tail serviceActivationMapping) targetName infrastructure targetProperty)
      else generateProfileManifest (tail serviceActivationMapping) targetName infrastructure targetProperty
  ;
  
  /*
   * Generates profiles for every machine that has services deployed on it, and
   * maps the profiles to the targets in the network.
   *
   * Parameters:
   * pkgs: Nixpkgs top-level expression
   * infrastructure: Infrastructure attributeset
   * targetNames: Names of the targets in the infrastructure attributeset
   * targetProperty: Attribute from the infrastructure model that is used to connect to the Disnix interface
   * serviceActivationMapping: List of activation mappings
   *
   * Returns:
   * List of machine profiles mapping to targets in the network
   */
   
  generateProfilesMapping = pkgs: infrastructure: targetNames: targetProperty: serviceActivationMapping:
    let
      target = getAttr (head targetNames) infrastructure;
      servicesPerTarget = queryServicesByTargetName serviceActivationMapping (head targetNames) infrastructure targetProperty;
      profileManifest = generateProfileManifest serviceActivationMapping (head targetNames) infrastructure targetProperty;
      mappingItem = {
        profile = (pkgs.buildEnv {
          name = head targetNames;
          paths = servicesPerTarget;
          manifest = pkgs.writeTextFile {
            name = "manifest";
            text = "${pkgs.lib.concatMapStrings (manifestItem: "${manifestItem}\n") profileManifest}";
          };
          ignoreCollisions = true;
        }).outPath;
        target = getTargetProperty targetProperty target;
      };
    in
    if targetNames == [] then []
    else [ mappingItem ] ++ generateProfilesMapping pkgs infrastructure (tail targetNames) targetProperty serviceActivationMapping
  ;
  
  /**
   * Generates a list of target properties from an infrastructure model.
   *
   * Parameters:
   * infrastructure: The infrastructure model, which is an attributeset containing targets in the network
   * targetProperty: Attribute from the infrastructure model that is used to connect to the Disnix interface
   * clientInterface: Path to the executable used to connect to the Disnix interface
   *
   * Returns:
   * List of target properties
   */
   
  generateTargetPropertyList = infrastructure: targetProperty: clientInterface:
    map (targetName:
      let
        target = getAttr targetName infrastructure;
      in
      target // {
        targetProperty = if target ? targetProperty then target.targetProperty else targetProperty;
        clientInterface = if target ? clientInterface then target.clientInterface else clientInterface;
        numOfCores = if target ? numOfCores then target.numOfCores else 1;
        system = if target ? system then target.system else builtins.currentSystem;
      }
    ) (attrNames infrastructure)
  ;
  
  /*
   * Returns the names of the services that have been distributed to a given target machine.
   *
   * Parameters:
   * distribution: Distribution attribute set
   * target: An attribute set representing a target machine from the infrastructure model
   *
   * Returns:
   * A list of strings containing the names services mapped to the given target
   */
  findServiceNamesPerTarget = distribution: target:
    filter (serviceName:
      let
        targets = getAttr serviceName distribution;
      in
      elem target targets
    ) (attrNames distribution)
  ;
  
  /*
   * Returns all the services that have been distributed to a given target machine.
   *
   * Parameters:
   * services: The final services model
   * distribution: Distribution attribute set
   * target: An attribute set representing a target machine from the infrastructure model
   *
   * Returns:
   * A list of attribute sets containing the services mapped to the given target
   */
  findServicesPerTarget = services: distribution: target:
    listToAttrs (map (serviceName: {
      name = serviceName;
      value = getAttr serviceName services;
    }) (findServiceNamesPerTarget distribution target))
  ;
  
  /*
   * Generates an inverse distribution attribute set, which is the infrastructure
   * model in which each target is augmented with a services attribute that is
   * a list of services that have been mapped to it.
   *
   * Parameters:
   * services: The final services model
   * infrastructure: The infrastructure model, which is an attributeset containing targets in the network
   * distribution: Distribution attribute set
   *
   * Returns:
   * An inverse distribution attribute set
   */
  generateInverseDistribution = services: infrastructure: distribution:
    pkgs.lib.mapAttrs (targetName: target:
      target // { services = findServicesPerTarget services distribution target; }
    ) infrastructure
  ;
  
  /*
   * Generates a manifest file consisting of a profile mapping, service
   * activation mapping, snapshots mapping and targets from the 3 Disnix models.
   *
   * Parameters:
   * pkgs: Nixpkgs top-level expression which contains the buildEnv function
   * servicesFun: The services model, which is a function that returns an attributeset of service declarations
   * infrastructure: The infrastructure model, which is an attributeset containing targets in the network
   * distributionFun: The distribution model, which is a function that returns an attributeset of services mapping to targets in the infrastructure model.
   * targetProperty: Attribute from the infrastructure model that is used to connect to the Disnix interface
   * clientInterface: Path to the executable used to connect to the Disnix interface
   * deployState: Indicates whether to globally deploy state
   *
   * Returns:
   * An attributeset which should be exported to XML representing the manifest
   */
   
  generateManifest = pkgs: servicesFun: infrastructure: distributionFun: targetProperty: clientInterface: deployState:
    let
      distribution = distributionFun { inherit infrastructure; };
      initialServices = servicesFun { inherit distribution invDistribution; system = null; inherit pkgs; };
      checkedServices = checkServiceNames initialServices;
      servicesWithTargets = augmentTargetsInDependsOn distribution checkedServices;
      servicesWithDistribution = evaluatePkgFunctions distribution invDistribution servicesWithTargets servicesFun "outPath" targetProperty;
      serviceActivationMapping = generateServiceActivationMapping (attrNames servicesWithDistribution) servicesWithDistribution targetProperty;
      snapshotsMapping = generateSnapshotsMapping (attrNames servicesWithDistribution) servicesWithDistribution deployState;
      invDistribution = generateInverseDistribution servicesWithDistribution infrastructure distribution;
    in
    { profiles = generateProfilesMapping pkgs infrastructure (attrNames infrastructure) targetProperty serviceActivationMapping;
      activation = filter (mapping: mapping.type != "package") serviceActivationMapping;
      snapshots = filter (mapping: mapping.type != "package") snapshotsMapping;
      targets = generateTargetPropertyList infrastructure targetProperty clientInterface;
    }
  ;
  
  /*
   * Generates a distributed derivation file constisting of a mapping of store derivations
   * to machines from the 3 Disnix models.
   *
   * Parameters:
   * servicesFun: The services model, which is a function that returns an attributeset of service declarations
   * infrastructure: The infrastructure model, which is an attributeset containing targets in the network
   * distributionFun: The distribution model, which is a function that returns an attributeset of
   * services mapping to targets in the infrastructure model.
   * targetProperty: Attribute from the infrastructure model that is used to connect to the Disnix interface
   * clientInterface: Path to the executable used to connect to the Disnix interface
   *
   * Returns: 
   * An attributeset which should be exported to XML representing the distributed derivation
   */

  generateDistributedDerivation = servicesFun: infrastructure: distributionFun: targetProperty: clientInterface:
    let
      distribution = distributionFun { inherit infrastructure; };
      initialServices = servicesFun { inherit distribution invDistribution; system = null; inherit pkgs; };
      checkedServices = checkServiceNames initialServices;
      servicesWithTargets = augmentTargetsInDependsOn distribution checkedServices;
      servicesWithDistribution = evaluatePkgFunctions distribution invDistribution servicesWithTargets servicesFun "drvPath" targetProperty;
      serviceActivationMapping = generateServiceActivationMapping (attrNames servicesWithDistribution) servicesWithDistribution targetProperty;
      targets = generateTargetPropertyList infrastructure targetProperty clientInterface;
      invDistribution = generateInverseDistribution servicesWithDistribution infrastructure distribution;
    in
    {
      build = map (mappingItem: { derivation = mappingItem.service; target = mappingItem.target; }) serviceActivationMapping;
      interfaces = map (target: { target = getTargetProperty targetProperty target; clientInterface = target.clientInterface; } ) targets;
    }
  ;
  
  /**
   * Generates a mapping of services to machines by iterating over the available service names,
   * and mapping them to a target name.
   *
   * Parameters:
   * serviceNames: List of service names
   * targetNames: List of target names onto which services are mapped
   * allTargetNames: List of all possible target names
   *
   * Returns:
   * A string with attributes mapping services to a target
   */
   
  generateDistributionModelBody = serviceNames: targetNames: allTargetNames:
    if serviceNames == [] then ""
    else
      "  ${head serviceNames} = [ infrastructure.${head targetNames} ];\n" +
      (if (tail targetNames) == [] then generateDistributionModelBody (tail serviceNames) allTargetNames allTargetNames
      else generateDistributionModelBody (tail serviceNames) (tail targetNames) allTargetNames)
  ;
  
  /**
   * Generates a distribution model from a given services model and infrastructure model by using
   * the roundrobin scheduling method.
   *
   * Parameters:
   * servicesFun: Services model, a function which returns an attributeset with service declarations
   * infrastructure: Infrastructure model, an attributeset capturing properties of machines in the network
   *
   * Returns:
   * A distribution model Nix expression
   */
  generateDistributionModelRoundRobin = servicesFun: infrastructure:
    let
      services = servicesFun {
        distribution = null;
        invDistribution = null;
        system = null;
        inherit pkgs;
      };
    in
    ''
      {infrastructure}:
      
      {
      ${generateDistributionModelBody (attrNames services) (attrNames infrastructure) (attrNames infrastructure)}}
    ''
  ;
}
