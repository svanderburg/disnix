#!/bin/bash -e

source common.bash

$disnix_manifest -s services-complete.nix -i infrastructure.nix -d distribution-simple.nix
