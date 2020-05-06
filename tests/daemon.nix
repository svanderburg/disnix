{nixpkgs, disnix}:

with import "${nixpkgs}/nixos/lib/testing.nix" { system = builtins.currentSystem; };

simpleTest {
  nodes = {
    machine = {pkgs, ...}:

    {
      environment.systemPackages = [ disnix ];
    };
  };
  testScript = ''
    $machine->mustSucceed("disnix-service --daemon");
    my $result = $machine->mustSucceed("disnix-client --print-invalid /nix/store/00000000000000000000000000000000-invalid");

    if((substr $result, 0, -1) eq "/nix/store/00000000000000000000000000000000-invalid") {
        print "We got the invalid path that we expect!\n";
    } else {
        die("We did not get the invalid path: ".$result);
    }
  '';
}
