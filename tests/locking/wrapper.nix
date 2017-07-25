{stdenv, dysnomia}:
{name ? "wrapper"}:

stdenv.mkDerivation {
  inherit name;
  buildCommand = ''
    mkdir -p $out/bin
    cat > $out/bin/wrapper << "EOF"
    #! ${stdenv.shell} -e
    
    source ${dysnomia}/share/dysnomia/util
    
    composeUtilityVariables "wrapper" "${name}" $3

    case "$1" in
        activate)
            markComponentAsActive
            ;;
        deactivate)
            markComponentAsGarbage
            ;;
        lock)
            exit $(cat /tmp/lock_status)
            ;;
        unlock)
            exit 0
            ;;
    esac
    EOF
    
    chmod +x $out/bin/wrapper
  '';
}
