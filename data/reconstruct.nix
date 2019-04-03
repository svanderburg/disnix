let
  inherit (builtins) sort head tail substring stringLength filter attrNames getAttr;

  /**
   * Reconstructs activation mappings from a list of services from a profile
   * manifest.
   *
   * Parameters:
   * target: An attribute set representing a target machine from the infrastructure model
   * services: A list of services originating from a profile manifest
   *
   * Returns:
   * A list of activation mappings
   */
  reconstructActivationMappingsFromServices = target: services:
    map (service:
      { inherit (service) service container name type dependsOn connectsTo _key;
        inherit target;
      }
    ) services;

  /**
   * Reconstructs activation mappings from a list of deployed services per
   * target machine.
   *
   * Parameters:
   * servicesPerTarget: A list of attribute sets containing the deployed services per machine.
   *
   * Returns:
   * A list of activation mappings
   */
  reconstructActivationMappings = servicesPerTarget:
    if servicesPerTarget == [] then []
    else
      let
        config = head servicesPerTarget;
        mappings = reconstructActivationMappingsFromServices (config.target) (config.services);
      in
      mappings ++ reconstructActivationMappings (tail servicesPerTarget);

  deriveComponentName = service:
     substring 33 (stringLength service) (baseNameOf service);

  /**
   * Reconstructs snapshot mappings from a list of services from a profile
   * manifest.
   *
   * Parameters:
   * target: An attribute set representing a target machine from the infrastructure model
   * services: A list of services originating from a profile manifest
   *
   * Returns:
   * A list of snapshot mappings
   */
  reconstructSnapshotMappingsFromServices = target: services:
    map (service:
      { inherit (service) service container type;
        inherit target;
        component = deriveComponentName (service.service);
      }
    ) services;

  /**
   * Reconstructs activation mappings from a list of deployed services per
   * target machine.
   *
   * Parameters:
   * servicesPerTarget: A list of attribute sets containing the deployed services per machine.
   *
   * Returns:
   * A list of activation mappings
   */
  reconstructSnapshotMappings = servicesPerTarget:
    if servicesPerTarget == [] then []
    else
      let
        statefulServices = filter (service: service.stateful) (config.services);
        config = head servicesPerTarget;
        mappings = reconstructSnapshotMappingsFromServices (config.target) statefulServices;
      in
      mappings ++ reconstructSnapshotMappings (tail servicesPerTarget);

  /**
   * Compares two activation mappings for ordering.
   *
   * Parameters:
   * left: An activation mapping
   * right: An activation mapping
   *
   * Returns:
   * true if left comes before right
   */
  compareActivationMappings = left: right:
    if left.name == right.name then
      if left.container == right.container then
        left.target < right.target
      else
        left.container < right.container
    else
      left.name < right.name;

  /**
   * Compares two snapshot mappings for ordering.
   *
   * Parameters:
   * left: A snapshot mapping
   * right: A snapshot mapping
   *
   * Returns:
   * true if left comes before right
   */
  compareSnapshotMappings = left: right:
    if left.component == right.component then
      if left.container == right.container then
        left.target < right.target
      else
        left.container < right.container
    else
      left.component < right.component;

  ## todo: duplicate
  normalizeTarget = {target, defaultClientInterface, defaultTargetProperty}:
    { clientInterface = defaultClientInterface;
      targetProperty = defaultTargetProperty;
      numOfCores = 1;
      system = builtins.currentSystem;
    } // target;

  /**
   * Reconstructs the manifest from the profile manifests of the target machines.
   *
   * Parameters:
   * infrastructure: Infrastructure attributeset
   * capturedProperties: A attribute set containing the captured properties and services per target
   * targetProperty: Attribute from the infrastructure model that is used to connect to the Disnix interface
   * clientInterface: Path to the executable used to connect to the Disnix interface
   *
   * Returns:
   * A manifest representation that can be transformed into XML
   */
  reconstructManifest = {infrastructure, capturedProperties, defaultTargetProperty, defaultClientInterface}:
    { inherit (capturedProperties) distribution;
      activation = sort compareActivationMappings (reconstructActivationMappings capturedProperties.servicesPerTarget);
      snapshots = sort compareSnapshotMappings (reconstructSnapshotMappings capturedProperties.servicesPerTarget);
      targets = map (targetName:
        let
          target = getAttr targetName infrastructure;
        in
        normalizeTarget {
          inherit target defaultClientInterface defaultTargetProperty;
        }
      ) (attrNames infrastructure);
    };
in
reconstructManifest
