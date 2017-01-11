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
            if [ ! -d /var/db/${name} ]
            then
                mkdir -p /var/db/${name}
                echo 0 > /var/db/${name}/state
            fi
            markComponentAsActive
            ;;
        deactivate)
            markComponentAsGarbage
            ;;
        snapshot)
            tmpdir=$(mktemp -d)
            cd $tmpdir
            
            cp /var/db/${name}/state .
            
            hash=$(cat state | sha256sum)
            hash=''${hash:0:64}
            
            snapshotsPath=$(composeSnapshotsPath)
            
            if [ -d $snapshotsPath/$hash ]
            then
                rm -Rf $tmpdir
            else
                mkdir -p $snapshotsPath/$hash
                mv state $snapshotsPath/$hash
                rmdir $tmpdir
            fi
            
            createGenerationSymlink $hash
            ;;
        restore)
            lastSnapshot=$(determineLastSnapshot)
        
            if [ "$lastSnapshot" != "" ]
            then
                cp $lastSnapshot/state /var/db/${name}
            fi
            ;;
        collect-garbage)
            if componentMarkedAsGarbage
            then
                rm /var/db/${name}/state
                rmdir /var/db/${name}
                unmarkComponentAsGarbage
            fi
            ;;
    esac
    EOF
    
    chmod +x $out/bin/wrapper
  '';
}
