{lib}:

let
  inherit (builtins) unsafeDiscardOutputDependency getAttr attrNames concatLists;

  # TODO: duplicate
  getTargetProperty = target:
    getAttr target.targetProperty target.properties;

  generateDerivationPerTargetGroupedByService = services:
    map (serviceName:
      let
        service = getAttr serviceName services;
      in
      map (mapping: { inherit (mapping) target; derivation = unsafeDiscardOutputDependency service._pkgsPerSystem."${mapping.system}".drvPath; }) service._systemsPerTarget
    ) (attrNames services);

  generateDerivationPerTarget = services:
    let
      derivationPerTargetGroupedByService = generateDerivationPerTargetGroupedByService services;
    in
    lib.unique (concatLists derivationPerTargetGroupedByService);

  generateDistributedDerivation = {architecture}:
    {
      build = generateDerivationPerTarget architecture.services;

      interfaces = map (targetName:
        let
          target = getAttr targetName architecture.infrastructure;
        in
        { target = getTargetProperty target;
          inherit (target) clientInterface;
        }) (attrNames architecture.infrastructure);
    };
in
generateDistributedDerivation
