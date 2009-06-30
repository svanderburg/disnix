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

  /* Checks whether the given service exists in the distribution model */
  checkServiceInDistribution = service: distribution:
    if distribution == [] then abort ("Service: "+service.name+" not found in distribution model!")
    else
      if service.name == (head distribution).service.name then service
      else checkServiceInDistribution service (tail distribution)
  ;
  
  /* Checks whether the given attribute set of inter-dependencies exist in the distribution model */
  checkInterDependenciesInDistribution = dependencies: distribution:
    listToAttrs (map (dependencyName:
      let dependency = getAttr dependencyName dependencies;
      in
        { name = dependencyName;
          value = checkServiceInDistribution dependency distribution;
	}
    ) (attrNames dependencies))
  ;

  /* Checks the distribution model whether all inter-dependencies are present for each service */  
  checkDistribution = distribution:
    if distribution == [] then []
    else
      let distributionItem = head distribution;
      in
        [ (distributionItem //
	    { service = distributionItem.service // 
	      { dependsOn = checkInterDependenciesInDistribution distributionItem.service.dependsOn distribution; };
	    })
	] ++ checkDistribution (tail distribution)
  ;
}
