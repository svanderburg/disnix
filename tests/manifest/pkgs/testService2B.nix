{stdenv}:
{testService1}:

stdenv.mkDerivation {
  name = "testService2B";
  buildCommand = ''
    (
    echo "testService2B"
    echo "Depends on service: ${testService1.name}"
    echo "Targets:"
    cat <<EOF
    ${stdenv.lib.concatMapStrings (target: "${target.hostname}\n") (testService1.targets)}
    EOF
    echo "Target: ${testService1.target.hostname}"
    ) > $out
  '';
}
