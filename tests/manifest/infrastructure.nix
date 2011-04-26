{
  testtarget1 = {
    hostname = "testtarget1";
    supportedTypes = [ "echo" "process" "wrapper" ];
    
    meta = {
      description = "The first test target";
    };
  };
  
  testtarget2 = {
    hostname = "testtarget2";
    supportedTypes = [ "echo" "process" "wrapper" ];
    
    meta = {
      description = "The second test target";
    };
  };
}
