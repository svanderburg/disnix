{lib}:

let
  inherit (builtins) attrNames getAttr;
in
rec {
  /*
   * Checks if a list normalizedTargets contains a mapping to a certain target
   * machine.
   *
   * Example:
   *   targetListContainsTarget { normalizedTargets = [ { target = "test1"; container = "process"; } ]; targetName = "test1"; }
   *   =>
   *   true
   */
  targetListContainsTarget = {normalizedTargets, targetName}:
    lib.any (targetReference: targetReference.target == targetName) normalizedTargets;

  /*
   * Generates for each target machine, a list of packages that should be deployed to
   * the machine by deriving the appropriate package attribute from every service.
   *
   * Example result:
   *   { test1 = [ /nix/store/....-pkg1 /nix/store/...-pkg2 ];
   *     test2 = ...
   *   }
   */
  generateTargetPackagesFromServices = {normalizedInfrastructure, normalizedServices}:
    lib.mapAttrs (targetName: target:
      let
        pkgProvidingServices = lib.filterAttrs (serviceName: normalizedService:
          if normalizedService ? targets
          then targetListContainsTarget {
            inherit targetName;
            normalizedTargets = normalizedService.targets;
          }
          else false
        ) normalizedServices;
      in
      map (serviceName:
        let
          normalizedService = getAttr serviceName normalizedServices;
        in
        normalizedService.pkgs."${target.system}"
      ) (attrNames pkgProvidingServices)
    ) normalizedInfrastructure;
}
