{stdenv, lib}:
{cyclicTestService1}:

stdenv.mkDerivation {
  name = "cyclicTestService2";
  buildCommand = ''
    mkdir -p $out
    (
    echo "cyclicTestService2"
    echo "Depends on service: ${cyclicTestService1.name}"
    echo "Targets:"
    cat <<EOF
    ${lib.concatMapStrings (target: "${target.properties.hostname}\n") (cyclicTestService1.targets)}
    EOF
    echo "Target: ${cyclicTestService1.target.properties.hostname}"
    ) > $out/config
  '';
}
