{lib}:

let
  inherit (builtins) attrNames getAttr listToAttrs;
in
rec {
  /*
   * Normalizes a target machine configuration by augmenting unspecified
   * configuration properties with default values.
   */
  normalizeTarget = {target, defaultClientInterface, defaultTargetProperty}:
    { clientInterface = defaultClientInterface;
      targetProperty = defaultTargetProperty;
      numOfCores = 1;
      system = builtins.currentSystem;
    } // target;

  /*
   * Normalizes all target machine configurations in the infrastructure model.
   */
  normalizeInfrastructure = {infrastructure, defaultClientInterface, defaultTargetProperty}:
    lib.mapAttrs (name: target: normalizeTarget {
      inherit target defaultClientInterface defaultTargetProperty;
    }) infrastructure;

  /*
   * Generates an attribute set that maps a unique connection property of a
   * machine to a machine name.
   *
   * Example:
   *   { test1 = {
   *       properties.hostname = "test1.local";
   *       targetProperty = "hostname";
   *     };
   *   }
   *   =>
   *   { "test1.local" = "test1";
   *   }
   */
  generateTargetAliases = {normalizedInfrastructure}:
    listToAttrs (map (targetName:
      let
        target = getAttr targetName normalizedInfrastructure;
      in
      { name = getAttr target.targetProperty target.properties; value = targetName; }
    ) (attrNames normalizedInfrastructure));
}
