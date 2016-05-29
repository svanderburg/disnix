{infrastructure}:

{
  testService1 = {
    targets = [ { target = infrastructure.testtarget1; } ];
  };
  testService2 = {
    targets = [ { target = infrastructure.testtarget2; } ];
  };
  testService3 = {
    targets = [ { target = infrastructure.testtarget2; } ];
  };
}
