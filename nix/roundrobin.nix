{pkgs}:

let
  inherit (builtins) head tail attrNames intersectAttrs functionArgs;

  /*
   * Generates a mapping of services to machines by iterating over the available service names,
   * and mapping them to a target name.
   *
   * Example:
   *   generateDistributionModelBody {
   *     serviceNames = [ "service1" "service2" ];
   *     targetNames = [ "target1" "target2" ];
   *     allTargetNames = [ "target1" "target2" ];
   *   }
   *   =>
   *   ''
   *     service1 = [ infrastructure.test1 ];
   *     service2 = [ infrastructure.test2 ];
   *   ''
   */

  generateDistributionModelBody = {serviceNames, targetNames, allTargetNames ? targetNames}:
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

  /*
   * Generates a distribution model from a given services model and infrastructure model by using
   * the roundrobin scheduling method.
   *
   * Parameters:
   * servicesFun: Services model, a function which returns an attributeset with service declarations
   * infrastructure: Infrastructure model, an attributeset capturing properties of machines in the network
   * extraParams: Optional extra parameters propagated to the services model
   *
   * Returns:
   * A distribution model Nix expression
   */
  generateDistributionModelRoundRobin = {servicesFun, infrastructure, extraParams}:
    let
      extraServiceParams = intersectAttrs (functionArgs servicesFun) extraParams;

      services = servicesFun ({
        distribution = null;
        invDistribution = null;
        system = null;
        inherit pkgs;
      } // extraServiceParams);
    in
    ''
      {infrastructure}:

      {
      ${generateDistributionModelBody {
        serviceNames = attrNames services;
        targetNames = attrNames infrastructure;
      }}}
    '';
in
generateDistributionModelRoundRobin
