{
  testtarget1 = {
    properties = {
      hostname = "testtarget1";
      supportedTypes = [ "echo" "process" "wrapper" ];

      meta = {
        description = "The first test target";
      };
    };

    containers = {
      echo = {
        hello = "hello-from-infrastructure-container";
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
}
