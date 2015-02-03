{stdenv}:

stdenv.mkDerivation {
  name = "testService1B";
  
  buildCommand = ''
    echo "testService1B" > $out
  '';
}
