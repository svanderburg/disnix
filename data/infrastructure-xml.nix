{ nixpkgs ? <nixpkgs>
, infrastructureFile
, targetProperty
, clientInterface
}:

let
  pkgs = import nixpkgs {};

  infrastructure = import infrastructureFile;

  normalizeInfrastructure = import ./normalize-infrastructure.nix {
    inherit (pkgs) lib;
  };
in
normalizeInfrastructure.normalizeInfrastructure {
  inherit infrastructure;
  defaultTargetProperty = targetProperty;
  defaultClientInterface = clientInterface;
}
