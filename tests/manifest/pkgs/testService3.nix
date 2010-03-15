{stdenv}:
{testService1, testService2}:

stdenv.mkDerivation {
  name = "testService3";
  buildCommand = ''
    ( 
    echo "testService3"
    echo "Depends on service: ${testService1.name}"
    echo "Targets:"
    cat <<EOF
    ${stdenv.lib.concatMapStrings (target: "${target.hostname}\n") (testService1.targets)}
    EOF
    echo "Target: ${testService1.target.hostname}"
      
    echo "Depends on service: ${testService2.name}"
    echo "Targets:"
    cat <<EOF
    ${stdenv.lib.concatMapStrings (target: "${target.hostname}\n") (testService2.targets)}
    EOF
    echo "Target: ${testService2.target.hostname}"
    ) > $out
  '';
}
