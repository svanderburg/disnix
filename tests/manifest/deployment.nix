let
  pkgs = import <nixpkgs> {};
in
{
  profiles = {
    testtarget1 = (pkgs.buildEnv {
      name = "testtarget1";
      paths = [
        pkgs.curl
      ];
    }).outPath;

    testtarget2 = (pkgs.buildEnv {
      name = "testtarget2";
      paths = [
        pkgs.strace
      ];
    }).outPath;
  };

  infrastructure = {
    testtarget1 = {
      properties = {
        hostname = "testtarget1";
      };
      targetProperty = "hostname";
      clientInterface = "disnix-ssh-client";
      system = builtins.currentSystem;
      numOfCores = 1;
    };
    testtarget2 = {
      properties = {
        hostname = "testtarget2";
      };
      targetProperty = "hostname";
      clientInterface = "disnix-ssh-client";
      system = builtins.currentSystem;
      numOfCores = 1;
    };
  };
}
