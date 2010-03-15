#!/bin/bash -e

source common.bash

$disnix_manifest -s services-incomplete.nix -i infrastructure.nix -d distribution-simple.nix

# The testcase should fail
if [ $? = 0 ]
then
    exit 1
else
    exit 0
fi
