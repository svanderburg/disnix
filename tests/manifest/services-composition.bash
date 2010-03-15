#!/bin/bash -e

source common.bash

manifest=`$disnix_manifest -s services-composition.nix -i infrastructure.nix -d distribution-composition.nix`

# Check whether testService2B is distributed

if [ "$(nix-store -qR $manifest | grep "\-testService2B")" = "" ]
then
    echo "ERROR: testService2B is not distributed!"
    exit 1
else
    exit 0
fi
