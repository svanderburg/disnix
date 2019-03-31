{lib, pkgs}:

let
  inherit (builtins) toXML filter attrNames getAttr hashString concatLists substring stringLength hasAttr;

  # TODO: duplicate
  getTargetProperty = target:
    getAttr target.targetProperty target.properties;

  generateServiceHashKey = mapping:
    hashString "sha256" (toXML {
      inherit (mapping) service name type dependsOn;
    });

  generateDependencyMappingsPerService = {services, dependencies}:
    map (dependencyName:
      let
        dependency = getAttr dependencyName dependencies;
        service = if dependency.type == "package"
          then throw "It is not allowed to refer to package: ${dependencyName} as an inter-dependency!"
          else getAttr dependency.name services;
      in
      map (target: {
        target = getTargetProperty target;
        container = target.selectedContainer;
        _key = generateServiceHashKey {
          inherit (service) name type;
          dependsOn = generateDependencyMappings { inherit services service; property = "dependsOn"; };
          service = service._pkgsPerSystem."${target.system}".outPath;
        };
      }) service.targets
    ) (attrNames dependencies);

  generateDependencyMappings = {services, service, property}:
    if hasAttr property service then
      let
        dependencies = service."${property}";
      in
      concatLists (generateDependencyMappingsPerService {
        inherit services dependencies;
      })
    else [];

  generateMappingPerTargetGroupedByService = services:
    let
      nonPackageServices = filter (name: services."${name}".type != "package") (attrNames services);
    in
    map (serviceName:
      let
        service = getAttr serviceName services;
      in
      map (target:
        let
          mapping = {
            inherit (service) name type;
            container = target.selectedContainer;
            target = getTargetProperty target;
            service = service._pkgsPerSystem."${target.system}".outPath;
            dependsOn = generateDependencyMappings { inherit services service; property = "dependsOn"; };
            connectsTo = generateDependencyMappings { inherit services service; property = "connectsTo"; };
          };
        in
        mapping // {
          _key = generateServiceHashKey mapping;
        }
      ) service.targets
    ) (nonPackageServices);

  generateActivationMappings = services:
    concatLists (generateMappingPerTargetGroupedByService services);

  generateSnapshotMappings = {activationMappings, services}:
    let
      statefulActivationMappings = filter (mapping:
        let
          service = services."${mapping.name}";
        in
        service ? deployState && service.deployState
      ) activationMappings;
    in
    map (activationMapping: {
      inherit (activationMapping) service type target container;
      component = substring 33 (stringLength activationMapping.service) (baseNameOf activationMapping.service);
    }) statefulActivationMappings;

  generateProfileManifest = {architecture, activation, target}:
    let
      activationMappingsToTarget = filter (mapping: mapping.target == target && mapping.type != "package") activation;
    in
    concatLists (map (mapping:
      let
        service = getAttr mapping.name architecture.services;

        stateful = if (service ? deployState && service.deployState) then "true" else "false";
        dependsOn = "[${toString (map (dependency: "{ target = \"${dependency.target}\"; container = \"${dependency.container}\"; _key = \"${dependency._key}\"; }") (mapping.dependsOn))}]";
        connectsTo = "[${toString (map (dependency: "{ target = \"${dependency.target}\"; container = \"${dependency.container}\"; _key = \"${dependency._key}\"; }") (mapping.connectsTo))}]";
        # TODO: refactor duplication
        #"
      in
      [ mapping.name mapping.service mapping.container mapping.type mapping._key stateful dependsOn connectsTo ]
    ) activationMappingsToTarget);

  generateProfilesFromPkgs = {architecture, activation}:
    map (targetName:
      let
        target = getAttr targetName architecture.infrastructure;
        paths = getAttr targetName architecture.pkgs;
      in
      {
        profile = (pkgs.buildEnv {
          name = targetName;
          inherit paths;
          ignoreCollisions = true;
          manifest = pkgs.writeTextFile {
            name = "manifest";
            text = lib.concatMapStrings (entry: "${entry}\n") (generateProfileManifest { inherit architecture activation; target = getTargetProperty target; });
          };
        }).outPath;

        target = getTargetProperty target;
      }
    ) (attrNames architecture.pkgs);

  generateManifest = {architecture}:
    rec {
      profiles = generateProfilesFromPkgs {
        inherit architecture activation;
      };

      activation = generateActivationMappings architecture.services;

      snapshots = generateSnapshotMappings {
        activationMappings = activation;
        inherit (architecture) services;
      };

      targets = map (targetName: architecture.infrastructure."${targetName}") (attrNames architecture.infrastructure);
    };
in
generateManifest
