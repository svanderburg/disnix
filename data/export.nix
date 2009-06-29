{servicesFile, infrastructureFile, distributionFile, mapFunFile, stdenv}:

let  
  lib = import ./lib.nix;
  services = import servicesFile { inherit distribution; };
  infrastructure = import infrastructureFile;
  distribution = import distributionFile { inherit services infrastructure; };
  mapFun = import mapFunFile;
  newServices = lib.mapDependenciesOnPackages (lib.mapTargetsOnServices services distribution);
  newDistribution = import distributionFile { services = newServices; inherit infrastructure; };  
  distributionXML = builtins.toXML (map mapFun newDistribution);  
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
