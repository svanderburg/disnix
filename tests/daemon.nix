{nixpkgs, disnix}:

with import "${nixpkgs}/nixos/lib/testing-python.nix" { system = builtins.currentSystem; };

simpleTest {
  nodes = {
    machine = {pkgs, ...}:

    {
      environment.systemPackages = [ disnix ];
    };
  };
  testScript = ''
    machine.succeed("disnix-service --daemon")
    result = machine.succeed(
        "disnix-client --print-invalid /nix/store/00000000000000000000000000000000-invalid"
    )

    if result[:-1] == "/nix/store/00000000000000000000000000000000-invalid":
        print("We got the invalid path that we expect!")
    else:
        raise Exception("We did not get the invalid path: {}".format(result))
  '';
}
