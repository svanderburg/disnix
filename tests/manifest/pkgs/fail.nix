{stdenv}:

stdenv.mkDerivation {
  name = "fail";
  buildCommand = ''
    mkdir -p $out/bin
    cat > $out/bin/wrapper <<EOF
    #! ${stdenv.shell} -e
    echo "FAILURE" >&2
    false
    EOF
    chmod +x $out/bin/wrapper
  '';
}
