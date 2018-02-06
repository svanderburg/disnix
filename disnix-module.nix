{pkgs, lib, config, ...}:

with lib;

let
  cfg = config.services.disnixTest;
in
{
  options = {
    services = {
      disnixTest = {
        enable = mkOption {
          type = types.bool;
          default = false;
          description = "Whether to enable Disnix";
        };

        enableMultiUser = mkOption {
          type = types.bool;
          default = true;
          description = "Whether to support multi-user mode by enabling the Disnix D-Bus service";
        };

        package = mkOption {
          type = types.path;
          description = "The Disnix package";
        };

        dysnomia = mkOption {
          type = types.path;
          description = "The Dysnomia package";
        };
      };
    };
  };

  config = mkIf cfg.enable {
    ids.gids = { disnix = 200; };
    users.extraGroups = [ { gid = 200; name = "disnix"; } ];

    services.dbus.enable = true;
    services.dbus.packages = [ cfg.package ];
    services.openssh.enable = true;

    services.disnixTest.package = mkDefault (import ./release.nix {}).build."${builtins.currentSystem}";

    systemd.services.disnix = mkIf cfg.enableMultiUser
      { description = "Disnix server";
        wantedBy = [ "multi-user.target" ];
        after = [ "dbus.service" ];

        path = [ config.nix.package cfg.package cfg.dysnomia ];
        environment = {
          HOME = "/root";
        }
        // (if config.environment.variables ? DYSNOMIA_CONTAINERS_PATH then { inherit (config.environment.variables) DYSNOMIA_CONTAINERS_PATH; } else {})
        // (if config.environment.variables ? DYSNOMIA_MODULES_PATH then { inherit (config.environment.variables) DYSNOMIA_MODULES_PATH; } else {});

        serviceConfig.ExecStart = "${cfg.package}/bin/disnix-service";
      };

    environment.systemPackages = [ cfg.package ];
  };
}
