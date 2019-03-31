{pkgs}:

let
  inherit (builtins) head tail attrNames;

  /**
   * Generates a mapping of services to machines by iterating over the available service names,
   * and mapping them to a target name.
   *
   * Parameters:
   * serviceNames: List of service names
   * targetNames: List of target names onto which services are mapped
   * allTargetNames: List of all possible target names
   *
   * Returns:
   * A string with attributes mapping services to a target
   */
   
  generateDistributionModelBody = {serviceNames, targetNames, allTargetNames}:
    if serviceNames == [] then ""
    else
      "  ${head serviceNames} = [ infrastructure.${head targetNames} ];\n" +
      (if (tail targetNames) == [] then generateDistributionModelBody {
        serviceNames = tail serviceNames;
        targetNames = allTargetNames;
        inherit allTargetNames;
      }
      else generateDistributionModelBody {
        serviceNames = tail serviceNames;
        targetNames = tail targetNames;
        inherit allTargetNames;
      });

  /**
   * Generates a distribution model from a given services model and infrastructure model by using
   * the roundrobin scheduling method.
   *
   * Parameters:
   * servicesFun: Services model, a function which returns an attributeset with service declarations
   * infrastructure: Infrastructure model, an attributeset capturing properties of machines in the network
   *
   * Returns:
   * A distribution model Nix expression
   */
  generateDistributionModelRoundRobin = {servicesFun, infrastructure}:
    let
      services = servicesFun {
        distribution = null;
        invDistribution = null;
        system = null;
        inherit pkgs;
      };
    in
    ''
      {infrastructure}:

      {
      ${generateDistributionModelBody {
        serviceNames = attrNames services;
        targetNames = attrNames infrastructure;
        allTargetNames = attrNames infrastructure;
      }}}
    '';
in
generateDistributionModelRoundRobin
