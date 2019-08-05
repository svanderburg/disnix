{stdenv}:

stdenv.mkDerivation {
  name = "testService1";
  
  buildCommand = ''
    mkdir -p $out
    echo "testService1" > $out/config
  '';
}
