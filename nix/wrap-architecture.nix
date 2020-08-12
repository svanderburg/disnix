{lib}:

let
  inherit (builtins) isAttrs isList elem hasAttr all attrNames intersectAttrs functionArgs;
in
rec {
  /*
   * Returns the infrastructure model, in which every target machine is
   * augmented with a service property referring an attribute set of
   * all services deployed to the target machine.
   *
   * Example result:
   *   {
   *     test1 = {
   *       properties.hostname = "test1";
   *       system = "x86_64-linux";
   *       services = {
   *         foo = {
   *           name = "foo";
   *           type = "process";
   *           ...
   *         };
   *       };
   *     };
   *   }
   */
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

  /*
   * Checks whether all keys in the distribution model correspond to a service
   * in the service model.
   */
  checkServiceReferencesInDistribution = {distribution, services}:
    all (serviceName: hasAttr serviceName services) (attrNames distribution);

  /*
   * Augments each service with their corresponding targets specifier in the
   * distribution model.
   *
   * If there is a reference to a non-existent service in the distribution model,
   * it throws an exception.
   */
  augmentTargetsToServices = {services, distribution}:
    if checkServiceReferencesInDistribution {
      inherit services distribution;
    } then
      lib.mapAttrs (name: service:
        if service ? targets then service else service // {
          targets = if hasAttr name distribution
            then distribution."${name}"
            else [];
        }
      ) services
    else throw "The distribution model contains a mapping of a service that is not in the services model!";

  /*
   * Takes the basic Disnix input models and wraps then into a single deployment
   * architecture model:
   *
   * - the infrastructure model becomes the infrastructure property
   * - the services model becomes the services property
   * - the distribution model mappings, get appended as targets to their corresponding services
   * - the packages model becomes the targetPackages property
   *
   * Partial example result:
   *   {pkgs, system}:
   *
   *   {
   *     services = {
   *       foo = {
   *         name = "foo";
   *         targets = ...;
   *         ...
   *       };
   *       ...
   *     };
   *     infrastructure = ...
   *     targetPackages = ...
   *   }
   */
  wrapBasicInputsModelsIntoArchitectureFun =
    {servicesFun ? null, infrastructure, distributionFun ? null, packagesFun ? null, extraParams}:

    {pkgs, system}:

    let
      extraServiceParams = intersectAttrs (functionArgs servicesFun) extraParams;
      extraDistributionParams = intersectAttrs (functionArgs distributionFun) extraParams;

      distribution = if distributionFun == null then {} else distributionFun ({
        inherit infrastructure;
      } // extraDistributionParams);

      services = if servicesFun == null then {} else servicesFun ({
        inherit pkgs system distribution;

        invDistribution = generateInverseDistribution {
          inherit services infrastructure distribution;
        };
      } // extraServiceParams);

      servicesWithTargets = augmentTargetsToServices {
        inherit services distribution;
      };

      targetPackages = if packagesFun == null then {} else packagesFun {
        inherit pkgs system;
      };
    in
    {
      services = servicesWithTargets;
      inherit infrastructure targetPackages;
    };
}
