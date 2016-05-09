{infrastructure}:

{
  testService1 = {
    targets = [ { target = infrastructure.testtarget1; container = "echo1"; } ];
  };
  testService2 = {
    targets = [ { target = infrastructure.testtarget2; container = "echo2"; } ];
  };
  testService3 = {
    targets = [
      { target = infrastructure.testtarget1; container = "echo1"; }
      { target = infrastructure.testtarget2; container = "echo2"; }
    ];
  };
}
