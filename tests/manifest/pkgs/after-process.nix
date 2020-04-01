{stdenv}:

stdenv.mkDerivation {
  name = "after-process";
  buildCommand = ''
    mkdir -p $out/bin
    cat > $out/bin/wrapper <<EOF
    #! ${stdenv.shell} -e
    [ "$(cat /tmp/process_out)" = "yes" ]
    EOF
    chmod +x $out/bin/wrapper
  '';
}
