/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2018  Sander van der Burg
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <defaultoptions.h>
#include "compare-manifest.h"

static void print_usage(const char *command)
{
    printf("Usage: %s [OPTION] MANIFEST\n\n", command);

    puts(
    "The command `disnix-compare' is used to parse and compare two manifest files.\n\n"

    "Options:\n"
    "  -o, --old-manifest=MANIFEST          Nix profile path where the manifest\n"
    "                                       should be stored, so that Disnix knows\n"
    "                                       the current configuration of a\n"
    "                                       distributed system. By default it is\n"
    "                                       stored in the profile directory of the\n"
    "                                       user.\n"
    "  -p, --profile=PROFILE                Name of the profile in which the services\n"
    "                                       are registered. Defaults to: default\n"
    "      --coordinator-profile-path=PATH  Path to the manifest of the previous\n"
    "                                       configuration. By default this tool will\n"
    "                                       use the manifest stored in the disnix\n"
    "                                       coordinator profile instead of the\n"
    "                                       specified one, which is usually sufficient\n"
    "                                       in most cases.\n"
    "  -h, --help                           Shows the usage of this command to the\n"
    "                                       user\n"

    "\nExit status:\n"
    " 0                                     The manifests are equal.\n"
    " 1                                     The manifests are not equal.\n"
    " 2                                     An error occured while processing the\n"
    "                                       manifests.\n"

    "\nEnvironment:\n"
    "  DISNIX_PROFILE       Sets the name of the profile that stores the manifest on\n"
    "                       the coordinator machine and the deployed services per\n"
    "                       machine on each target (Defaults to: default)\n"

    );
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"coordinator-profile-path", required_argument, 0, DISNIX_OPTION_COORDINATOR_PROFILE_PATH},
        {"profile", required_argument, 0, DISNIX_OPTION_PROFILE},
        {"old-manifest", required_argument, 0, DISNIX_OPTION_OLD_MANIFEST},
        {"help", no_argument, 0, DISNIX_OPTION_HELP},
        {"version", no_argument, 0, DISNIX_OPTION_VERSION},
        {0, 0, 0, 0}
    };

    char *manifest_file;
    char *old_manifest = NULL;
    char *profile = NULL;
    char *coordinator_profile_path = NULL;

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "m:o:p:hv", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case DISNIX_OPTION_PROFILE:
                profile = optarg;
                break;
            case DISNIX_OPTION_COORDINATOR_PROFILE_PATH:
                coordinator_profile_path = optarg;
                break;
            case DISNIX_OPTION_OLD_MANIFEST:
                old_manifest = optarg;
                break;
            case DISNIX_OPTION_HELP:
                print_usage(argv[0]);
                return 0;
            case DISNIX_OPTION_VERSION:
                print_version(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    /* Validate options */

    profile = check_profile_option(profile);

    if(optind >= argc)
        manifest_file = NULL;
    else
        manifest_file = argv[optind];

    return compare_manifest(manifest_file, old_manifest, coordinator_profile_path, profile); /* Execute compare operation */
}
