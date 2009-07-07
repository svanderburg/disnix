{servicesFile, infrastructureFile, stdenv}:

let
  lib = import ./lib.nix;
  services = import servicesFile { distribution = null; };
  infrastructure = import infrastructureFile;
  inherit (builtins) attrNames;
in
stdenv.mkDerivation {
  name = "distributionModel";
  buildCommand = ''
    ensureDir $out
    cat > $out/distribution.nix <<EOF
    {services, infrastructure}:
    
    [
    ${lib.generateDistributionModelBody (attrNames services) (attrNames infrastructure) (attrNames infrastructure)}]
    EOF
  '';
}
