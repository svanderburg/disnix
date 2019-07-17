{dysnomia, disnix, enableMultiUser ? true}:
{config, pkgs, ...}:

{
  imports = [ ../disnix-module.nix ];

  virtualisation.writableStore = true;
  virtualisation.pathsInNixDB = [ pkgs.stdenv ] ++ pkgs.libxml2.all ++ pkgs.libxslt.all;

  services.disnixTest = {
    enable = true;
    package = disnix;
    inherit dysnomia enableMultiUser;
  };

  users.extraUsers = [
    { uid = 1000;
      name = "unprivileged";
      group = "users";
      shell = "/bin/sh";
      description = "Unprivileged user for the disnix-service";
    }

    { uid = 1001;
      name = "privileged";
      group = "users";
      shell = "/bin/sh";
      extraGroups = [ "disnix" ];
      description = "Privileged user for the disnix-service";
    }
  ];
  
  # We can't download any substitutes in a test environment. To make tests
  # faster, we disable substitutes so that Nix does not waste any time by
  # attempting to download them.
  nix.extraOptions = ''
    substitute = false
  '';

  environment.systemPackages = [ dysnomia pkgs.libxml2 ];

  environment.etc."dysnomia/properties" = {
    source = pkgs.writeTextFile {
      name = "dysnomia-properties";
      text = ''
        foo=bar
        supportedTypes=("process" "wrapper")
      '';
    };
  };
}
