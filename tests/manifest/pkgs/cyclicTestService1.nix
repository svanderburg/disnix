{stdenv, lib}:
{cyclicTestService2}:

stdenv.mkDerivation {
  name = "cyclicTestService1";
  buildCommand = ''
    mkdir -p $out
    (
    echo "cyclicTestService1"
    echo "Depends on service: ${cyclicTestService2.name}"
    echo "Targets:"
    cat <<EOF
    ${lib.concatMapStrings (target: "${target.properties.hostname}\n") (cyclicTestService2.targets)}
    EOF
    echo "Target: ${cyclicTestService2.target.properties.hostname}"
    ) > $out/config
  '';
}
