{ servicesFile
, infrastructureFile
, extraParams ? {}
, nixpkgs ? <nixpkgs>
}:

let 
  servicesFun = import servicesFile;
  infrastructure = import infrastructureFile;

  pkgs = import nixpkgs {};

  generateDistributionModelRoundRobin = import ./roundrobin.nix {
    inherit pkgs;
  };
in
pkgs.stdenv.mkDerivation {
  name = "distribution.nix";
  buildCommand = ''
    cat > $out <<EOF
    ${generateDistributionModelRoundRobin {
      inherit servicesFun infrastructure extraParams;
    }}
    EOF
  '';
}
