rec {
  inherit (builtins) attrNames getAttr listToAttrs head tail unsafeDiscardOutputDependency;
  
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
   * Attributeset with service declarations augumented with targets in the dependsOn attribute
   */
   
  augumentTargetsInDependsOn = distribution: services:
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
   * Service attributeset augumented with a distribution mapping property 
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
	      pkg = (getAttr serviceName (servicesFun { inherit distribution; inherit system; })).pkg;
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
   * services: Services attributeset, augumented with distributions
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
	  inherit (service) type;
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
   * Generates profiles for every machine that has services deployed on it, and
   * maps the profiles to the targets in the network.
   *
   * Parameters:
   * pkgs: Nixpkgs top-level expression which contains the buildEnv function
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
      mappingItem = {
        profile = (pkgs.buildEnv {
	  name = head targetNames;
	  paths = queryServicesByTargetName serviceActivationMapping (head targetNames) infrastructure;
	}).outPath;
	target = getAttr targetProperty target;
      };
    in
    if targetNames == [] then []
    else [ mappingItem ] ++ generateProfilesMapping pkgs infrastructure (tail targetNames) targetProperty serviceActivationMapping
  ;
  
  /*
   * Generates a distribution export file constisting of a profile mapping and
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
   * An attributeset which should be exported to XML representing the distribution export
   */
   
  generateDistributionExport = pkgs: servicesFun: infrastructure: distributionFun: targetProperty:
    let
      distribution = distributionFun { inherit infrastructure; };
      initialServices = servicesFun { inherit distribution; system = null; };
      servicesWithTargets = augumentTargetsInDependsOn distribution initialServices;
      servicesWithDistribution = evaluatePkgFunctions distribution servicesWithTargets servicesFun "outPath";
      serviceActivationMapping = generateServiceActivationMapping (attrNames servicesWithDistribution) servicesWithDistribution targetProperty;
    in
    { profiles = generateProfilesMapping pkgs infrastructure (attrNames infrastructure) targetProperty serviceActivationMapping;
      activation = serviceActivationMapping;
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
      initialServices = servicesFun { inherit distribution; system = null; };
      servicesWithTargets = augumentTargetsInDependsOn distribution initialServices;
      servicesWithDistribution = evaluatePkgFunctions distribution servicesWithTargets servicesFun "drvPath";
      serviceActivationMapping = generateServiceActivationMapping (attrNames servicesWithDistribution) servicesWithDistribution targetProperty;
    in
    map (mappingItem: { derivation = mappingItem.service; target = getAttr targetProperty (mappingItem.target); }) serviceActivationMapping
  ;
}
