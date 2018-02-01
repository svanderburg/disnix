{stdenv}:
{cyclicTestService2}:

stdenv.mkDerivation {
  name = "cyclicTestService1";
  buildCommand = ''
    (
    echo "cyclicTestService1"
    echo "Depends on service: ${cyclicTestService2.name}"
    echo "Targets:"
    cat <<EOF
    ${stdenv.lib.concatMapStrings (target: "${target.properties.hostname}\n") (cyclicTestService2.targets)}
    EOF
    echo "Target: ${cyclicTestService2.target.properties.hostname}"
    ) > $out
  '';
}
