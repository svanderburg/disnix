{disnix, dysnomia}:
{config, pkgs, ...}:

{
  virtualisation.writableStore = true;
  virtualisation.pathsInNixDB = [ pkgs.stdenv ];
  
  ids.gids = { disnix = 200; };
  users.extraGroups = [ { gid = 200; name = "disnix"; } ];
  
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

  services.dbus.enable = true;
  services.dbus.packages = [ disnix ];
  services.openssh.enable = true;
  
  jobs.disnix =
    { description = "Disnix server";
      wantedBy = [ "multi-user.target" ];
      after = [ "dbus.service" ];
      
      path = [ pkgs.nix pkgs.getopt disnix dysnomia ];
      environment = {
        HOME = "/root";
      };

      exec = "disnix-service";
    };
    
    environment.systemPackages = [ disnix dysnomia ];
}
