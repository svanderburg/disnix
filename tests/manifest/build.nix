let
  pkgs = import <nixpkgs> {};
in
{
  derivationMappings = [
    (builtins.unsafeDiscardOutputDependency (pkgs.buildEnv {
      name = "testtarget1";
      paths = [
        pkgs.curl
      ];
    }).drvPath)

    (builtins.unsafeDiscardOutputDependency (pkgs.buildEnv {
      name = "testtarget2";
      paths = [
        pkgs.strace
      ];
    }).drvPath)
  ];

  interfaces = {
    testtarget1 = {
      targetAddress = "testtarget1";
      clientInterface = "disnix-ssh-client";
    };
    testtarget2 = {
      targetAddress = "hostname";
      clientInterface = "disnix-ssh-client";
    };
  };
}
