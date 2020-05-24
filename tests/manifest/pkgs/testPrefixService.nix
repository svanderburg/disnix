{stdenv}:
{prefix}:

stdenv.mkDerivation {
  name = prefix;

  buildCommand = ''
    mkdir -p $out
    echo "${prefix}" > $out/config
  '';
}
