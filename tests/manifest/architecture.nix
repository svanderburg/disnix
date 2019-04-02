{system, pkgs}:

let
  customPkgs = import ./pkgs { inherit pkgs system; };
in
rec {
  targetPackages = {
    testtarget1 = [
      pkgs.curl
    ];
    testtarget2 = [
      pkgs.strace
    ];
  };

  services = rec {
    testService1 = {
      name = "testService1";
      pkg = customPkgs.testService1;
      type = "echo";

      targets = [ infrastructure.testtarget1 ];
    };

    testService2 = {
      name = "testService2";
      pkg = customPkgs.testService2;
      dependsOn = {
        inherit testService1;
      };
      type = "echo";

      targets = [ infrastructure.testtarget2 ];
    };

    testService3 = {
      name = "testService3";
      pkg = customPkgs.testService3;
      dependsOn = {
        inherit testService1 testService2;
      };
      type = "echo";

      targets = [ infrastructure.testtarget2 ];
    };
  };

  infrastructure = {
    testtarget1 = {
      properties = {
        hostname = "testtarget1";
        supportedTypes = [ "echo" "process" "wrapper" ];

        meta = {
          description = "The first test target";
        };
      };
    };

    testtarget2 = {
      properties = {
        hostname = "testtarget2";
        supportedTypes = [ "echo" "process" "wrapper" ];

        meta = {
          description = "The second test target";
        };
      };
    };
  };
}
