{lib, pkgs}:
{normalizedArchitecture}:

let
  inherit (builtins) attrNames getAttr toXML hashString listToAttrs stringLength substring unsafeDiscardStringContext elem filter;

  generateHash = {name, type, pkg, dependsOn}:
    unsafeDiscardStringContext (hashString "sha256" (toXML {
      inherit name type pkg dependsOn;
    }));

  getTargetProperty = target:
    getAttr target.targetProperty target.properties;

  generateDependencyMappingsForOrderedDependencies = {service, services, targetAliases}:
    (generateDependencyMappings {
      inherit service services targetAliases;
      dependencyType = "dependsOn";
    }) ++ (generateDependencyMappings {
      inherit service services targetAliases;
      dependencyType = "activatesAfter";
    });

  generateDependencyMappings = {services, service, dependencyType, targetAliases}:
    lib.flatten (map (dependencyName:
      let
        dependency = getAttr dependencyName service."${dependencyType}";
      in
      map (target:
        let
          targetProperty = getTargetProperty target;
          dependencyService = if dependency.type == "package"
            then throw "It is not allowed to refer to package: ${dependency.name} as an inter-dependency!"
            else getAttr dependency.name services;
        in
        {
          service = generateHash {
            inherit (dependencyService) name type;
            pkg = dependencyService._pkgsPerSystem."${target.system}".outPath;

            dependsOn = generateDependencyMappingsForOrderedDependencies {
              inherit services targetAliases;
              service = dependencyService;
            };
          };
          container = target.selectedContainer;
          target = getAttr targetProperty targetAliases;
        }
      ) dependency.targets
    ) (attrNames service."${dependencyType}"));

  generateServicesPerSystem = {services, service, targetAliases}:
    map (system:
      let
        pkg = service._pkgsPerSystem."${system}".outPath;

        dependsOn = generateDependencyMappingsForOrderedDependencies {
          inherit service services targetAliases;
        };
      in
      {
        name = generateHash {
          inherit (service) name type;
          inherit pkg dependsOn;
        };

        value = {
          inherit (service) name type;
          inherit pkg dependsOn;
          connectsTo = generateDependencyMappings {
            inherit services service targetAliases;
            dependencyType = "connectsTo";
          };
        } // lib.optionalAttrs (service ? providesContainers) {
          inherit (service) providesContainers;
        };
      }
    ) (attrNames service._pkgsPerSystem);

  generateTargetAliases = {infrastructure}:
    listToAttrs (map (targetName:
      let
        target = getAttr targetName infrastructure;
      in
      { name = getTargetProperty target; value = targetName; }
    ) (attrNames infrastructure));

  generateServiceMappingsPerSystem = {services, service, targetAliases}:
    map (target:
      let
        targetProperty = getTargetProperty target;
        targetName = getAttr targetProperty targetAliases;

        dependsOn = generateDependencyMappingsForOrderedDependencies {
          inherit service services targetAliases;
        };
      in
      {
        service = generateHash {
          inherit (service) name type;
          inherit dependsOn;
          pkg = service._pkgsPerSystem."${target.system}".outPath;
        };
        target = targetName;
        container = target.selectedContainer;
      }
    ) service.targets;

  generateSnapshotMappingsPerSystem = {services, service, targetAliases}:
    map (target:
      let
        targetProperty = getTargetProperty target;
        targetName = getAttr targetProperty targetAliases;

        dependsOn = generateDependencyMappingsForOrderedDependencies {
          inherit service services targetAliases;
        };

        pkg = service._pkgsPerSystem."${target.system}".outPath;
      in
      {
        service = generateHash {
          inherit (service) name type;
          inherit dependsOn pkg;
        };
        component = substring 33 (stringLength pkg) (baseNameOf pkg);
        container = target.selectedContainer;
        target = targetName;
      }
    ) service.targets;

  targetAliases = generateTargetAliases {
    inherit (normalizedArchitecture) infrastructure;
  };

  filterServicesPerTarget = {services, infrastructure, targetName, targetAliases}:
    lib.filterAttrs (serviceName: service:
      let
        serviceTargetProperties = map (target: getTargetProperty target) service.targets;
        targetProperty = getTargetProperty (infrastructure."${targetName}");
      in
      elem targetProperty serviceTargetProperties
    ) services;

  generateServices = {services, targetAliases}:
    let
      nonPackageServiceNames = filter (serviceName:
        let
          service = getAttr serviceName services;
        in
        service.type != "package"
      ) (attrNames services);
    in
    listToAttrs (lib.flatten (map (serviceName:
      let
        service = getAttr serviceName services;
      in
      generateServicesPerSystem {
        inherit services service targetAliases;
      }
    ) nonPackageServiceNames));

  filterLocalTargets = {mappings, targetName}:
    map (mapping:
      if mapping.target == targetName
      then removeAttrs mapping [ "target" ]
      else mapping
    ) mappings;

  generateProfileManifest = {targetName, services, serviceMappings, snapshotMappings}:
    let
      localServiceMappings = filter (serviceMapping: serviceMapping.target == targetName) serviceMappings;
      localSnapshotMappings = filter (snapshotMapping: snapshotMapping.target == targetName) snapshotMappings;

      localServiceHashes = map (serviceMapping: serviceMapping.service) localServiceMappings;
      localServices = lib.filterAttrs (hash: service:
        elem hash localServiceHashes
      ) services;
    in
    {
      services = lib.mapAttrs (hash: service:
        service
        // lib.optionalAttrs (service ? dependsOn) {
          dependsOn = filterLocalTargets {
            mappings = service.dependsOn;
            inherit targetName;
          };
        }
        // lib.optionalAttrs (service ? connectsTo) {
          connectsTo = filterLocalTargets {
            mappings = service.connectsTo;
            inherit targetName;
          };
        }
      ) localServices;

      serviceMappings = filterLocalTargets {
        mappings = localServiceMappings;
        inherit targetName;
      };
      snapshotMappings = filterLocalTargets {
        mappings = localSnapshotMappings;
        inherit targetName;
      };
    };

  services = generateServices {
    inherit targetAliases;
    inherit (normalizedArchitecture) services;
  };

  serviceMappings =
    let
      nonPackageServiceNames = filter (serviceName:
        let
          service = getAttr serviceName normalizedArchitecture.services;
        in
        service.type != "package"
      ) (attrNames normalizedArchitecture.services);
    in
    lib.flatten (map (serviceName:
      let
        service = getAttr serviceName normalizedArchitecture.services;
      in
      generateServiceMappingsPerSystem {
        inherit (normalizedArchitecture) services;
        inherit service targetAliases;
      }
    ) nonPackageServiceNames);

  snapshotMappings =
    let
      statefulServiceNames = filter (serviceName:
        let
          service = getAttr serviceName normalizedArchitecture.services;
        in
        service.type != "package" && service.deployState
      ) (attrNames normalizedArchitecture.services);
    in
    lib.flatten (map (serviceName:
    generateSnapshotMappingsPerSystem {
      service = getAttr serviceName normalizedArchitecture.services;
      inherit (normalizedArchitecture) services;
      inherit targetAliases;
    }
  ) statefulServiceNames);
in
{
  profiles = lib.mapAttrs (targetName: paths:
    let
      profileManifest = generateProfileManifest {
        inherit targetName services serviceMappings snapshotMappings;
      };

      profileManifestPkg = pkgs.stdenv.mkDerivation {
        name = "profilemanifest";
        buildInputs = [ pkgs.libxslt ];
        manifestXML = builtins.toXML profileManifest;
        passAsFile = [ "manifestXML" ];

        buildCommand = ''
          mkdir -p $out

          if [ "$manifestXMLPath" != "" ]
          then
              xsltproc ${generateProfileManifestXSL} $manifestXMLPath > $out/profilemanifest.xml
          else
          (
          cat <<EOF
          $manifestXML
          EOF
          ) | xsltproc ${generateProfileManifestXSL} - > $out/profilemanifest.xml
          fi
        '';
      };

      generateProfileManifestXSL = ./generateprofilemanifest.xsl;
    in
    (pkgs.buildEnv {
      name = targetName;
      paths = [ profileManifestPkg ] ++ paths;
      ignoreCollisions = true;
    }).outPath
  ) normalizedArchitecture.targetPackages;

  inherit services serviceMappings snapshotMappings;
  inherit (normalizedArchitecture) infrastructure;
}
