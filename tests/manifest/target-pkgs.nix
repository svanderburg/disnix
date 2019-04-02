{system, pkgs}:

{
  testtarget1 = [
    pkgs.curl
  ];
  testtarget2 = [
    pkgs.strace
  ];
}
