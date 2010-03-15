{stdenv}:
{testService1}:

stdenv.mkDerivation {
  name = "testService2";
  buildCommand = ''
    (
    echo "testService2"
    echo "Depends on service: ${testService1.name}"
    echo "Targets:"
    cat <<EOF
    ${stdenv.lib.concatMapStrings (target: "${target.hostname}\n") (testService1.targets)}
    EOF
    echo "Target: ${testService1.target.hostname}"
    ) > $out
  '';
}
