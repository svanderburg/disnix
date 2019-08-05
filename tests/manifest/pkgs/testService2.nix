{stdenv}:
{testService1}:

stdenv.mkDerivation {
  name = "testService2";
  buildCommand = ''
    mkdir -p $out
    (
    echo "testService2"
    echo "Depends on service: ${testService1.name}"
    echo "Targets:"
    cat <<EOF
    ${stdenv.lib.concatMapStrings (target: "${target.properties.hostname}\n") (testService1.targets)}
    EOF
    echo "Target: ${testService1.target.properties.hostname}"
    ) > $out/config
  '';
}
