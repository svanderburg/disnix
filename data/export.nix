{servicesFile, infrastructureFile, distributionFile, serviceProperty, targetProperty, stdenv}:

let  
  lib = import ./lib.nix;
  services = import servicesFile { inherit distribution; };
  infrastructure = import infrastructureFile;
  distribution = import distributionFile { inherit services infrastructure; };
  newServices = lib.mapDependenciesOnPackages (lib.mapTargetsOnServices services distribution);
  newDistribution = import distributionFile { services = newServices; inherit infrastructure; };
  distributionXML = builtins.toXML (lib.generateDistributionExport newDistribution newServices serviceProperty targetProperty);
in
stdenv.mkDerivation {
  name = "distributionExport";
  buildCommand = ''
    ensureDir $out
    cat > $out/export.xml <<EOF
    ${distributionXML}
    EOF
  '';
}
