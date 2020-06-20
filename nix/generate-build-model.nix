{lib, pkgs}:

let
  inherit (builtins) attrNames concatLists getAttr unsafeDiscardOutputDependency;
in
rec {
  /*
   * Converts a normalized infrastructure model into an attribute set of
   * interface specifications only exposing connectivity properties.
   *
   * Example:
   *   {
   *     test1 = {
   *       properties.hostname = "test1.local";
   *       targetProperty = "hostname";
   *       clientInterface = "disnix-ssh-client";
   *     };
   *
   *     test2 = {
   *       properties.hostname = "test2.local";
   *       targetProperty = "hostname";
   *       clientInterface = "disnix-ssh-client";
   *     };
   *   }
   *   =>
   *   {
   *     test1 = {
   *       targetAddress = "test1.local";
   *       clientInterface = "disnix-ssh-client";
   *     };
   *
   *     test2 = {
   *       targetAddress = "test2.local";
   *       clientInterface = "disnix-ssh-client";
   *     };
   *   }
   */
  generateInterfaces = {normalizedInfrastructure}:
    lib.mapAttrs (targetName: normalizedTarget: {
      targetAddress = normalizedTarget.properties."${normalizedTarget.targetProperty}";
      inherit (normalizedTarget) clientInterface;
    }) normalizedInfrastructure;

  /*
   * Generates a list of Nix store derivations for all packages mapped to a
   * target machine.
   */
  generateDerivationMappingsPerTarget = {packages, targetName}:
    map (package: {
      derivation = unsafeDiscardOutputDependency package.drvPath;
      interface = targetName;
    }) packages;

  /*
   * Generates an attribute in which Nix store derivations of all packages are
   * mapped to target machine in the network.
   *
   * Example result:
   *   [
   *     { derivation = "/nix/store/aaaaaa...-foo.drv"; interface = "test1"; }
   *     { derivation = "/nix/store/aaaaaa...-bar.drv"; interface = "test2"; }
   *   ]
   */
  generateDerivationMappings = {targetPackages}:
    concatLists (map (targetName:
      let
        packages = getAttr targetName targetPackages;
      in
      generateDerivationMappingsPerTarget {
        inherit packages targetName;
      }
    ) (attrNames targetPackages));

  /*
   * Generates a build model from a normalized architecture model that maps
   * Nix store derivations of every package to target machines in the network.
   */
  generateBuildModel = {normalizedArchitecture}:
    {
      interfaces = generateInterfaces {
        normalizedInfrastructure = normalizedArchitecture.infrastructure;
      };

      derivationMappings = generateDerivationMappings {
        inherit (normalizedArchitecture) targetPackages;
      };
    };
}
