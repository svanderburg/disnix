{infrastructure}:

{
  testService1 = [ infrastructure.testTarget1 infrastructure.testTarget2 ]; # Multiple targets
  testService2 = [ infrastructure.testTarget1 infrastructure.testTarget2 ]; # Multiple targets
  testService3 = [ infrastructure.testTarget1 ];
}
