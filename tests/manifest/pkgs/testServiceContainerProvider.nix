{stdenv}:

stdenv.mkDerivation {
  name = "testServiceContainerProvider";
  buildCommand = ''
    mkdir -p $out/bin

    cat > $out/bin/wrapper << "EOF"
    #! ${stdenv.shell} -e
    true
    EOF
    chmod +x $out/bin/wrapper

    mkdir -p $out/libexec/dysnomia
    # This echo module is deliberately different from the one that comes with
    # Dysnomia. It will write the environment variables to a temp file.
    cat > $out/libexec/dysnomia/echo << "EOF"
    #! ${stdenv.shell} -e
    set > /tmp/echo_output
    EOF
    chmod +x $out/libexec/dysnomia/echo
  '';
}
