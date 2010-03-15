{stdenv}:

stdenv.mkDerivation {
  name = "testService1";
  
  buildCommand = ''
    echo "testService1" > $out
  '';
}
