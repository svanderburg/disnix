rec {
  /* Inherit some builtin functions for simplicity */
  inherit (builtins) head tail getAttr attrNames listToAttrs;
  
  /* Extracts a list of machines from the distribution model to where the given service is distributed */
  getTargets = service: distribution:
    if distribution == [] then []
    else
      if (head distribution).service.name == service.name then [ (head distribution).target ] ++ getTargets service (tail distribution)	
      else getTargets service (tail distribution)
  ;
  
  /* Extracts the first target from the distribution model to where the given service is distributed */
  getTarget = service: distribution:
    if getTargets service distribution == [] then null else head (getTargets service distribution)
  ;
   
  /* Searches for each dependency of a service the targets in the distribution model and adds them as targets and target attributes */
  mapTargetsOnDependencies = dependencies: distribution:
    listToAttrs (map (dependencyName:
      let dependency = getAttr dependencyName dependencies;
      in
        { name = dependencyName;
          value = dependency // 
            { targets = getTargets dependency distribution; target = getTarget dependency distribution; };
	}
      ) (attrNames dependencies))
  ;
  
  /* Maps the targets from the distribution model on each inter dependency of the service */
  mapTargetsOnServices = services: distribution:
    listToAttrs (map (serviceName: 
      let service = getAttr serviceName services;
      in
        { name = serviceName;
          value = service // { dependsOn = mapTargetsOnDependencies service.dependsOn distribution; };
	}
      ) (attrNames services))
  ;
  
  /* Passes the inter-dependencies as function arguments to the build functions of each service */
  mapDependenciesOnPackages = services:
    listToAttrs (map (serviceName: 
      let service = getAttr serviceName services;
      in
        { name = serviceName;
          value = service // { pkg = if service.dependsOn == {} then service.pkg else service.pkg service.dependsOn; };
        }
     ) (attrNames services))
  ;

  /*
   * Generates a distribution export by mapping the Nix store paths of each dependsOn attribute to each distribution
   * item and substituting the target property by a protocol specific property
   */     
  generateDistributionExport = distribution: services: serviceProperty: targetProperty:
    map (distributionItem:
          { service = getAttr serviceProperty distributionItem.service.pkg;
	    target = getAttr targetProperty distributionItem.target;
            dependsOn = 
	      map (dependencyName:
	        let serviceName = (getAttr dependencyName (distributionItem.service.dependsOn)).name;
		in 
	          getAttr serviceProperty (getAttr serviceName services).pkg
		) 
		(attrNames distributionItem.service.dependsOn);
	    type = distributionItem.service.type;
          }
        ) distribution
  ;
  
  /*
   * Generates the body of a distribution Nix expression, which iterates over all services
   * and devides the over the machines in the infrastructure model in equal proportions and order
   */
  generateDistributionModelBody = serviceNames: targets: allTargets:
    if targets == [] then generateDistributionModelBody serviceNames allTargets allTargets
    else
      if serviceNames == [] then ""
      else "  { service = services."+(head serviceNames)+"; target = infrastructure."+(head targets)+"; }\n" +
        generateDistributionModelBody (tail serviceNames) (tail targets) allTargets
  ;
}
