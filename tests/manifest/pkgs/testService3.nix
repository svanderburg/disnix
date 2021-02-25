{stdenv, lib}:
{testService1, testService2}:

stdenv.mkDerivation {
  name = "testService3";
  buildCommand = ''
    mkdir -p $out
    (
    echo "testService3"
    echo "Depends on service: ${testService1.name}"
    echo "Targets:"
    cat <<EOF
    ${lib.concatMapStrings (target: "${target.properties.hostname}\n") (testService1.targets)}
    EOF
    echo "Target: ${testService1.target.properties.hostname}"

    echo "Depends on service: ${testService2.name}"
    echo "Targets:"
    cat <<EOF
    ${lib.concatMapStrings (target: "${target.properties.hostname}\n") (testService2.targets)}
    EOF
    echo "Target: ${testService2.target.properties.hostname}"
    ) > $out/config
  '';
}
