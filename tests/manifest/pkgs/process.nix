{stdenv}:

stdenv.mkDerivation {
  name = "process";
  buildCommand = ''
    mkdir -p $out/bin
    cat > $out/bin/wrapper <<EOF
    #! ${stdenv.shell} -e
    echo yes > /tmp/process_out
    EOF
    chmod +x $out/bin/wrapper
  '';
}
