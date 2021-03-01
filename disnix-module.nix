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

        enableProfilePath = mkOption {
          type = types.bool;
          default = false;
          description = "Whether to expose Disnix profiles in the system's PATH";
        };

        profiles = mkOption {
          type = types.listOf types.string;
          default = [ "default" ];
          example = [ "default" ];
          description = "Names of the Disnix profiles to expose in the system's PATH";
        };
      };
    };
  };

  config = mkIf cfg.enable {
    ids.gids = { disnix = 200; };
    users.extraGroups.disnix.gid = 200;

    services.dbus.enable = true;
    services.dbus.packages = [ cfg.package ];
    services.openssh.enable = true;

    services.disnixTest.package = mkDefault (import ./release.nix {}).build."${pkgs.stdenv.system}";

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
    environment.variables.PATH = lib.optionals cfg.enableProfilePath (map (profileName: "/nix/var/nix/profiles/disnix/${profileName}/bin" ) cfg.profiles);
    environment.variables.DISNIX_REMOTE_CLIENT = lib.optionalString (cfg.enableMultiUser) "disnix-client";
  };
}
