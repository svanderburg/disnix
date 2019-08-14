{lib}:

let
  normalizeTarget = {target, defaultClientInterface, defaultTargetProperty}:
    { clientInterface = defaultClientInterface;
      targetProperty = defaultTargetProperty;
      numOfCores = 1;
      system = builtins.currentSystem;
    } // target;

  normalizeInfrastructure = {infrastructure, defaultClientInterface, defaultTargetProperty}:
    lib.mapAttrs (name: target: normalizeTarget {
      inherit target defaultClientInterface defaultTargetProperty;
    }) infrastructure;
in
{
  inherit normalizeTarget normalizeInfrastructure;
}
