{pkgs, lib}:

let
  inherit (builtins) elem filter;
in
rec {
  /*
   * Filters out all mappings mapped to a given target machine.
   *
   * Example:
   *   {
   *     mappings = [
   *       { service = "hash1..."; container = "process"; target = "test1"; }
   *       { service = "hash2..."; container = "process"; target = "test2"; }
   *     ];
   *     targetName = "test1";
   *   {
   *   =>
   *   [
   *     { service = "hash1..."; container = "process"; target = "test1"; }
   *   ]
   */
  filterLocalTargets = {mappings, targetName}:
    map (mapping:
      if mapping.target == targetName
      then removeAttrs mapping [ "target" ]
      else mapping
    ) mappings;

  /*
   * Generate a profile manifest XML configuration. It is identical to a
   * manifest XML file, except that it does not store the machine configurations
   * and it only captures the configuration properties of the machine it is
   * deployed to.
   *
   * This model can be used to reconstruct a deployment model from configurations
   * deployed on the target machines themselves.
   */
  generateProfileManifest = {targetName, deploymentModel}:
    let
      localServiceMappings = filter (serviceMapping: serviceMapping.target == targetName) deploymentModel.serviceMappings;
      localSnapshotMappings = filter (snapshotMapping: snapshotMapping.target == targetName) deploymentModel.snapshotMappings;

      localServiceHashes = map (serviceMapping: serviceMapping.service) localServiceMappings;
      localServices = lib.filterAttrs (hash: service: elem hash localServiceHashes) deploymentModel.services;
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

  /*
   * Generates Nix profiles for each target machine in the network containing
   * all Nix store paths that are required on a target machine, with a profile
   * manifest that describes the deployment configuration for that particular
   * machine.
   */
  generateProfiles = {targetPackages, deploymentModel}:
    lib.mapAttrs (targetName: paths:
      let
        profileManifest = generateProfileManifest {
          inherit targetName deploymentModel;
        };

        generateProfileManifestXSL = ./generateprofilemanifest.xsl;

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
      in
      (pkgs.buildEnv {
        name = targetName;
        paths = [ profileManifestPkg ] ++ paths;
        ignoreCollisions = true;
      }).outPath
    ) targetPackages;
}
