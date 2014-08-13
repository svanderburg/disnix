{nixpkgs, pkgs}:

let
  /* Defines all variants of Nixpkgs */
  pkgs_i686_linux = import nixpkgs { system = "i686-linux"; };
  pkgs_x86_64_linux = import nixpkgs { system = "x86_64-linux"; };
  pkgs_i686_darwin = import nixpkgs { system = "i686-darwin"; };
  pkgs_x86_64_darwin = import nixpkgs { system = "x86_64-darwin"; };
  pkgs_i686_freebsd = import nixpkgs { system = "i686-freebsd"; };
  pkgs_x86_64_freebsd = import nixpkgs { system = "x86_64-freebsd"; };
  pkgs_i686_cygwin = import nixpkgs { system = "i686-cygwin"; };
  
in
rec {
  inherit (builtins) attrNames getAttr listToAttrs head tail unsafeDiscardOutputDependency;

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
    if system == "i686-linux" then pkgs_i686_linux
    else if system == "x86_64-linux" then pkgs_x86_64_linux
    else if system == "i686-darwin" then pkgs_i686_darwin
    else if system == "x86_64-darwin" then pkgs_x86_64_darwin
    else if system == "i686-freebsd" then pkgs_i686_freebsd
    else if system == "x86_64-freebsd" then pkgs_x86_64_freebsd
    else if system == "i686-cygwin" then pkgs_i686_cygwin
    else abort "unsupported system type: ${system}";
  
  /*
   * Iterates over each service in the distribution attributeset, adds the according service
   * declaration in the attributeset and augements the targets of every inter-dependency
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
              in
              { name = argName;
                value = dependency // {
                  inherit targets;
                  target = head targets;
                };
              }
            ) (attrNames (service.dependsOn)))
          else {};
        };
      }
    ) (attrNames distribution))
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
   * services: Services attribute set, augemented with targets in dependsOn
   * servicesFun: Function that returns a services attributeset (defined in the services.nix file)
   * serviceProperty: Defines which property we need of a derivation (either "outPath" or "drvPath")
   *
   * Returns:
   * Service attributeset augmented with a distribution mapping property 
   */
   
  evaluatePkgFunctions = distribution: services: servicesFun: serviceProperty:
    listToAttrs (map (serviceName: 
      let
        targets = getAttr serviceName distribution;
        service = getAttr serviceName services;
      in
      { name = serviceName;
        value = service // {
          distribution = map (target:
            let
              system = if target ? system then target.system else builtins.currentSystem;
              pkg = (getAttr serviceName (servicesFun { inherit distribution; inherit system; pkgs = selectPkgs system; })).pkg;
            in
            { service = 
                if serviceProperty == "outPath" then
                  if service.dependsOn == {} then pkg.outPath
                  else (pkg (service.dependsOn)).outPath
                else
                  if service.dependsOn == {} then unsafeDiscardOutputDependency (pkg.drvPath)
                  else unsafeDiscardOutputDependency ((pkg (service.dependsOn)).drvPath)
                ;
              inherit target;
            }
          ) targets;
        };
      } 
    ) (attrNames distribution))
  ;

  /*
   * Maps a list of inter-dependency declarations to a list of inter-dependency
   * distributions.
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
      serviceName = (getAttr (head argNames) dependsOn).name;
      service = getAttr serviceName services;
      inherit (service) distribution;
    in
    if argNames == [] then [] else distribution ++ generateDependencyMapping (tail argNames) dependsOn services
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
      mappingItem = map (distributionItem:
        { inherit (distributionItem) service target;
          inherit (service) name type;
          inherit targetProperty;
          dependsOn = generateDependencyMapping (attrNames (service.dependsOn)) (service.dependsOn) services; 
        }
      ) (service.distribution);
    in
    if serviceNames == [] then [] else mappingItem ++ (generateServiceActivationMapping (tail serviceNames) services targetProperty)
  ;
  
  /*
   * Iterates over a service activation mapping list and filters out all the
   * services that are distributed to a specific target.
   *
   * Parameters:
   * serviceActivationMapping: List of activation mappings
   * targetName: Name of the target in the infrastructure model to filter on 
   * infrastructure: Infrastructure attributeset
   *   
   * Returns:
   * List of services that are distributed to the given target name
   */
   
  queryServicesByTargetName = serviceActivationMapping: targetName: infrastructure:
    if serviceActivationMapping == [] then []
    else
      if (head serviceActivationMapping).target == getAttr targetName infrastructure
      then [ (head serviceActivationMapping).service ] ++ (queryServicesByTargetName (tail serviceActivationMapping) targetName infrastructure)
      else queryServicesByTargetName (tail serviceActivationMapping) targetName infrastructure
  ;

  /*
   * Iterates over a service activation mapping list and filters out all the
   * services and types that are distributed to a specific target.
   *
   * Parameters:
   * serviceActivationMapping: List of activation mappings
   * targetName: Name of the target in the infrastructure model to filter on 
   * infrastructure: Infrastructure attributeset
   *   
   * Returns:
   * List of services and types that are distributed to the given target name
   */
  
  generateProfileManifest = serviceActivationMapping: targetName: infrastructure:
    if serviceActivationMapping == [] then []
    else
      if (head serviceActivationMapping).target == getAttr targetName infrastructure
      then [ (head serviceActivationMapping).service (head serviceActivationMapping).type ] ++ (generateProfileManifest (tail serviceActivationMapping) targetName infrastructure)
      else generateProfileManifest (tail serviceActivationMapping) targetName infrastructure
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
      servicesPerTarget = queryServicesByTargetName serviceActivationMapping (head targetNames) infrastructure;
      profileManifest = generateProfileManifest serviceActivationMapping (head targetNames) infrastructure;
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
        target = getAttr targetProperty target;
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
   *
   * Returns:
   * List of target properties
   */
   
  generateTargetPropertyList = infrastructure: targetProperty:
    map (targetName:
      let target = getAttr targetName infrastructure;
      in
      { targetProperty = getAttr targetProperty target;
        numOfCores = if target ? "numOfCores" then target.numOfCores else 1;
      }
    ) (attrNames infrastructure)
  ;
  
  /*
   * Generates a manifest file consisting of a profile mapping and
   * service activation mapping from the 3 Disnix models.
   *
   * Parameters:
   * pkgs: Nixpkgs top-level expression which contains the buildEnv function
   * servicesFun: The services model, which is a function that returns an attributeset of service declarations
   * infrastructure: The infrastructure model, which is an attributeset containing targets in the network
   * distributionFun: The distribution model, which is a function that returns an attributeset of
   * services mapping to targets in the infrastructure model.
   * targetProperty: Attribute from the infrastructure model that is used to connect to the Disnix interface
   *
   * Returns: 
   * An attributeset which should be exported to XML representing the manifest
   */
   
  generateManifest = pkgs: servicesFun: infrastructure: distributionFun: targetProperty:
    let
      distribution = distributionFun { inherit infrastructure; };
      initialServices = servicesFun { inherit distribution; system = null; inherit pkgs; };
      servicesWithTargets = augmentTargetsInDependsOn distribution initialServices;
      servicesWithDistribution = evaluatePkgFunctions distribution servicesWithTargets servicesFun "outPath";
      serviceActivationMapping = generateServiceActivationMapping (attrNames servicesWithDistribution) servicesWithDistribution targetProperty;
    in
    { profiles = generateProfilesMapping pkgs infrastructure (attrNames infrastructure) targetProperty serviceActivationMapping;
      activation = serviceActivationMapping;
      targets = generateTargetPropertyList infrastructure targetProperty;
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
   *
   * Returns: 
   * An attributeset which should be exported to XML representing the distributed derivation
   */

  generateDistributedDerivation = servicesFun: infrastructure: distributionFun: targetProperty:
    let
      distribution = distributionFun { inherit infrastructure; };
      initialServices = servicesFun { inherit distribution; system = null; inherit pkgs; };
      servicesWithTargets = augmentTargetsInDependsOn distribution initialServices;
      servicesWithDistribution = evaluatePkgFunctions distribution servicesWithTargets servicesFun "drvPath";
      serviceActivationMapping = generateServiceActivationMapping (attrNames servicesWithDistribution) servicesWithDistribution targetProperty;
    in
    map (mappingItem: { derivation = mappingItem.service; target = getAttr targetProperty (mappingItem.target); }) serviceActivationMapping
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
      services = servicesFun { distribution = null; system = null; inherit pkgs; };
    in
    ''
      {infrastructure}:
      
      {
      ${generateDistributionModelBody (attrNames services) (attrNames infrastructure) (attrNames infrastructure)}}
    ''
  ;
}
