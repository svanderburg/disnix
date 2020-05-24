{pkgs, system}:

rec {
  testService1 = import ./testService1.nix {
    inherit (pkgs) stdenv;
  };

  testService1B = import ./testService1B.nix {
    inherit (pkgs) stdenv;
  };

  testService2 = import ./testService2.nix {
    inherit (pkgs) stdenv;
  };

  testService2B = import ./testService2B.nix {
    inherit (pkgs) stdenv;
  };

  testService3 = import ./testService3.nix {
    inherit (pkgs) stdenv;
  };

  process = import ./process.nix {
    inherit (pkgs) stdenv;
  };

  fail = import ./fail.nix {
    inherit (pkgs) stdenv;
  };

  cyclicTestService1 = import ./cyclicTestService1.nix {
    inherit (pkgs) stdenv;
  };

  cyclicTestService2 = import ./cyclicTestService2.nix {
    inherit (pkgs) stdenv;
  };

  afterProcess = import ./after-process.nix {
    inherit (pkgs) stdenv;
  };

  testServiceContainerProvider = import ./testServiceContainerProvider.nix {
    inherit (pkgs) stdenv;
  };

  testServiceContainerConsumer = import ./testServiceContainerConsumer.nix {
    inherit (pkgs) stdenv;
  };

  testPrefixService = import ./testPrefixService.nix {
    inherit (pkgs) stdenv;
  };
}
