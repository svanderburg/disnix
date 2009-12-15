{ servicesFile
, infrastructureFile
}:

let 
  servicesFun = import servicesFile;
  infrastructure = import infrastructureFile;
  
  pkgs = import (builtins.getEnv "NIXPKGS_ALL") {};
  lib = import ./lib.nix;
in
pkgs.stdenv.mkDerivation {
  name = "distribution.nix";
  buildCommand = ''
    cat > $out <<EOF
    ${lib.generateDistributionModelRoundRobin servicesFun infrastructure}
    EOF
  '';
}
