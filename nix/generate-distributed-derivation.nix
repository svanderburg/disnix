{lib}:

let
  inherit (builtins) unsafeDiscardOutputDependency getAttr attrNames concatLists;

  # TODO: duplicate
  getTargetProperty = target:
    getAttr target.targetProperty target.properties;

  generateDerivationMappingsPerTarget = {packages, targetName}:
    map (package: { derivation = unsafeDiscardOutputDependency package.drvPath; interface = targetName; }) packages;

  generateDerivationMappings = {targetPackages}:
    concatLists (map (targetName:
      let
        packages = getAttr targetName targetPackages;
      in
      generateDerivationMappingsPerTarget {
        inherit packages targetName;
      }
    ) (attrNames targetPackages));

  generateDistributedDerivation = {architecture}:
    {
      #build = generateDerivationPerTarget architecture.services;
      derivationMappings = generateDerivationMappings {
        inherit (architecture) targetPackages;
      };

      interfaces = lib.mapAttrs (targetName: target:
        { targetAddress = getTargetProperty target;
          inherit (target) clientInterface;
        }
      ) architecture.infrastructure;
    };
in
generateDistributedDerivation
