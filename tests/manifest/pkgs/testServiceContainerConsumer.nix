{stdenv}:

stdenv.mkDerivation {
  name = "testServiceContainerConsumer";
  buildCommand = ''
    mkdir -p $out/bin
    cat > $out/bin/wrapper << "EOF"
    #! ${stdenv.shell} -e
    true
    EOF
    chmod +x $out/bin/wrapper
  '';
}
