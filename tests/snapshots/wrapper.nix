{stdenv, dysnomia}:

stdenv.mkDerivation {
  name = "wrapper";
  buildCommand = ''
    mkdir -p $out/bin
    cat > $out/bin/wrapper << "EOF"
    #! ${stdenv.shell} -e
    
    source ${dysnomia}/share/dysnomia/util
    
    determineComponentName "wrapper"
    checkStateDir
    determineTypeIdentifier "wrapper"
    composeSnapshotsPath
    composeGarbagePath
    composeGenerationsPath
    
    case "$1" in
        activate)
            mkdir -p /var/db/wrapper
            echo 0 > /var/db/wrapper/state
            unmarkStateAsGarbage
            ;;
        deactivate)
            markStateAsGarbage
            ;;
        snapshot)
            tmpdir=$(mktemp -d)
            cd $tmpdir
            
            cp /var/db/wrapper/state .
            
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
                cp $snapshotsPath/$lastSnapshot/state /var/db/wrapper
            fi
            ;;
        collect-garbage)
            if [ -f $garbagePath ]
            then
                rm /var/db/wrapper/state
                rmdir /var/db/wrapper
            fi
            ;;
    esac
    EOF
    
    chmod +x $out/bin/wrapper
  '';
}
