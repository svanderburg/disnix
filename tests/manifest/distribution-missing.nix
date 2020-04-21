{infrastructure}:

{
  testService1 = [ infrastructure.testtarget1 ];
  testService2 = [ infrastructure.testtarget2 ];
  missing = [ infrastructure.testtarget2 ]; # This service does not exist in the services model
}
