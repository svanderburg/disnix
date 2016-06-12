{pkgs, system, distribution, invDistribution}:

rec {
  package = {
    name = "package";
    pkg = pkgs.curl;
    type = "package";
  };
  
  service = {
    name = "service";
    pkg =
      {package}:
      
      pkgs.stdenv.mkDerivation {
        name = "service";
        buildCommand = ''
          mkdir -p $out/bin
          cat > $out/bin/service <<EOF
          #! ${pkgs.stdenv.shell} -e
          echo "service"
          EOF
          chmod +x $out/bin/service
        '';
      };
    dependsOn = {
      inherit package;
    };
    type = "echo";
  };
}
