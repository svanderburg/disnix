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
  
  environment.systemPackages = [ dysnomia ];

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
