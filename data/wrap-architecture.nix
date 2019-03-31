{lib}:

let
  inherit (builtins) isAttrs isList elem hasAttr;

  generateInverseDistribution = {services, infrastructure, distribution}:
    let
      convertMappingsListToTargetsList = mappings:
        map (mapping: mapping.target) mappings;

      findServicesPerTarget = {target}:
        lib.filterAttrs (name: targets:
          if hasAttr name distribution then
            if isList distribution."${name}" then elem target distribution."${name}"
            else if isAttrs distribution."${name}" then elem target (convertMappingsListToTargetsList distribution."${name}".targets)
            else throw "Unknown mapping type for service: ${name} in the distribution model!"
          else false
        ) services;
    in
    lib.mapAttrs (targetName: target:
      target // {
        services = findServicesPerTarget {
          inherit target;
        };
      }
    ) infrastructure;

  augmentTargetsToServices = {services, distribution}:
    lib.mapAttrs (name: service:
      if service ? targets then service else service // {
        targets = if hasAttr name distribution then distribution."${name}"
          else [];
      }
    ) services;

  wrapArchitectureModel = {servicesFun, infrastructure, distributionFun}:
    {pkgs, system}:

    let
      distribution = distributionFun {
        inherit infrastructure;
      };

      services = servicesFun {
        inherit pkgs system;
        inherit distribution;

        invDistribution = generateInverseDistribution {
          inherit services infrastructure distribution;
        };
      };

      servicesWithTargets = augmentTargetsToServices {
        inherit services distribution;
      };
    in
    {
      services = servicesWithTargets;
      inherit infrastructure;
    };
in
wrapArchitectureModel
