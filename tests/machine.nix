{dysnomia, disnix, enableMultiUser ? true, enableProfilePath ? false}:
{config, pkgs, ...}:

{
  imports = [ ../disnix-module.nix ];

  virtualisation.writableStore = true;
  virtualisation.additionalPaths = [ pkgs.stdenv pkgs.stdenvNoCC ] ++ pkgs.coreutils.all ++ pkgs.libxml2.all ++ pkgs.libxslt.all;

  services.disnixTest = {
    enable = true;
    package = disnix;
    inherit dysnomia enableMultiUser enableProfilePath;
  };

  users.extraUsers = {
    unprivileged = {
      uid = 1000;
      group = "users";
      shell = "/bin/sh";
      description = "Unprivileged user for the disnix-service";
      isNormalUser = true;
    };

    privileged = {
      uid = 1001;
      group = "users";
      shell = "/bin/sh";
      extraGroups = [ "disnix" ];
      description = "Privileged user for the disnix-service";
      isNormalUser = true;
    };
  };

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
