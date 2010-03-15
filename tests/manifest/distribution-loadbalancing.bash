#!/bin/bash -e

source common.bash

exitStatus=0

# Create manifest from expressions

manifest=`$disnix_manifest -s services-complete.nix -i infrastructure.nix -d distribution-loadbalancing.nix`
manifestClosure=`nix-store -qR $manifest`

# Determine target profiles

for i in $manifestClosure
do
    target1Profile=$(echo $i | grep "\-testTarget1")
    
    if [ "$target1Profile" != "" ]
    then
        break
    fi
done

for i in $manifestClosure
do
    target2Profile=$(echo $i | grep "\-testTarget2")
    
    if [ "$target2Profile" != "" ]
    then
        break
    fi
done

# Check whether testService1 is distributed to testTarget1 and testTarget2

if [ "$(nix-store -qR $target1Profile | grep "\-testService1")" = "" ]
then
    echo "ERROR: testService1 is not distributed to testTarget1!"
    exitStatus=1
fi

if [ "$(nix-store -qR $target2Profile | grep "\-testService1")" = "" ]
then
    echo "ERROR: testService1 is not distributed to testTarget2!"
    exitStatus=1
fi

# Check whether testService2 is distributed to testTarget1 and testTarget2

if [ "$(nix-store -qR $target1Profile | grep "\-testService2")" = "" ]
then
    echo "ERROR: testService2 is not distributed to testTarget1!"
    exitStatus=1
fi

if [ "$(nix-store -qR $target2Profile | grep "\-testService2")" = "" ]
then
    echo "ERROR: testService2 is not distributed to testTarget2!"
    exitStatus=1
fi

# Check whether testService3 is distributed only to testTarget1

if [ "$(nix-store -qR $target1Profile | grep "\-testService3")" = "" ]
then
    echo "ERROR: testService3 is not distributed to testTarget1!"
    exitStatus=1
fi

if [ "$(nix-store -qR $target2Profile | grep "\-testService3")" != "" ]
then
    echo "ERROR: testService3 should not be distributed to testTarget2!"
    exitStatus=1
fi

# Return exit status
exit $exitStatus
