{stdenv, dysnomia}:
{name ? "wrapper"}:

stdenv.mkDerivation {
  inherit name;
  buildCommand = ''
    mkdir -p $out/bin
    cat > $out/bin/wrapper << "EOF"
    #! ${stdenv.shell} -e
    
    source ${dysnomia}/share/dysnomia/util
    
    determineComponentName "${name}"
    checkStateDir
    determineTypeIdentifier "wrapper"
    composeSnapshotsPath
    composeGarbagePath
    composeGenerationsPath
    
    case "$1" in
        activate)
            if [ ! -d /var/db/${name} ]
            then
                mkdir -p /var/db/${name}
                echo 0 > /var/db/${name}/state
            fi
            unmarkStateAsGarbage
            ;;
        deactivate)
            markStateAsGarbage
            ;;
        snapshot)
            tmpdir=$(mktemp -d)
            cd $tmpdir
            
            cp /var/db/${name}/state .
            
            hash=$(cat state | sha256sum)
            hash=''${hash:0:64}
            
            if [ -d $snapshotsPath/$hash ]
            then
                rm -Rf $tmpdir
            else
                mkdir -p $snapshotsPath/$hash
                mv state $snapshotsPath/$hash
                rmdir $tmpdir
            fi
            
            createGenerationSymlink $snapshotsPath/$hash
            ;;
        restore)
            determineLastSnapshot
        
            if [ "$lastSnapshot" != "" ]
            then
                cp $snapshotsPath/$lastSnapshot/state /var/db/${name}
            fi
            ;;
        collect-garbage)
            if [ -f $garbagePath ]
            then
                rm /var/db/${name}/state
                rmdir /var/db/${name}
            fi
            ;;
    esac
    EOF
    
    chmod +x $out/bin/wrapper
  '';
}
