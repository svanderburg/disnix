{stdenv}:

stdenv.mkDerivation {
  name = "testService1B";

  buildCommand = ''
    mkdir -p $out
    echo "testService1B" > $out/config
  '';
}
