{ servicesFile
, infrastructureFile
, nixpkgs ? <nixpkgs>
}:

let 
  servicesFun = import servicesFile;
  infrastructure = import infrastructureFile;
  
  pkgs = import nixpkgs {};
  lib = import ./lib.nix { inherit pkgs nixpkgs; };
in
pkgs.stdenv.mkDerivation {
  name = "distribution.nix";
  buildCommand = ''
    cat > $out <<EOF
    ${lib.generateDistributionModelRoundRobin servicesFun infrastructure}
    EOF
  '';
}
