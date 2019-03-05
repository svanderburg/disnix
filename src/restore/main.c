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
#include "run-restore.h"

static void print_usage(const char *command)
{
    printf("Usage: %s [OPTION] [MANIFEST]\n\n", command);

    puts(
    "Restores the state of components deployed in a network of machines.\n\n"

    "By default, this command only restores the state of components that are recently\n"
    "activated, i.e. the ones that are defined in the current deployment manifest,\n"
    "but not the previous one.\n\n"

    "A full restore of the state of the entire system can be done by providing the\n"
    "--no-upgrade parameter.\n\n"

    "If no manifest has been provided, the last deployed one is used.\n\n"

    "Options:\n"
    "  -c, --container=CONTAINER            Name of the container in which the\n"
    "                                       mutable component is deployed\n"
    "  -C, --component=COMPONENT            Name of the mutable component to take\n"
    "                                       snapshots from\n"
    "  -o, --old-manifest=MANIFEST          Nix profile path where the manifest\n"
    "                                       should be stored, so that Disnix knows\n"
    "                                       the current configuration of a\n"
    "                                       distributed system. By default it is\n"
    "                                       stored in the profile directory of the\n"
    "                                       user.\n"
    "      --no-upgrade                     Indicates that no upgrade should be\n"
    "                                       performed and the state of all components\n"
    "                                       should be restored.\n"
    "      --transfer-only                  Transfers the snapshot from the target\n"
    "                                       machines, but does not actually restore\n"
    "                                       them\n"
    "      --depth-first                    Snapshots components depth-first as\n"
    "                                       opposed to breadth-first. This approach\n"
    "                                       is more space efficient, but slower.\n"
    "      --all                            Transfers all snapshot generations of the\n"
    "                                       target machines, not the latest\n"
    "      --keep=NUM                       Amount of snapshot generations to keep.\n"
    "                                       Defaults to: 1\n"
    "  -p, --profile=PROFILE                Name of the profile in which the services\n"
    "                                       are registered. Defaults to: default\n"
    "      --coordinator-profile-path=PATH  Path to the manifest of the previous\n"
    "                                       configuration. By default this tool will\n"
    "                                       use the manifest stored in the disnix\n"
    "                                       coordinator profile instead of the\n"
    "                                       specified one, which is usually sufficient\n"
    "                                       in most cases.\n"
    "  -m, --max-concurrent-transfers=NUM   Maximum amount of concurrent closure\n"
    "                                       transfers. Defauls to: 2\n"
    "  -h, --help                           Shows the usage of this command to the\n"
    "                                       user\n"

    "\nEnvironment:\n"
    "  DISNIX_PROFILE    Sets the name of the profile that stores the manifest on the\n"
    "                    coordinator machine and the deployed services per machine on\n"
    "                    each target (Defaults to: default)\n"
    );
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"container", required_argument, 0, 'c'},
        {"component", required_argument, 0, 'C'},
        {"coordinator-profile-path", required_argument, 0, DISNIX_OPTION_COORDINATOR_PROFILE_PATH},
        {"profile", required_argument, 0, 'p'},
        {"old-manifest", required_argument, 0, 'o'},
        {"transfer-only", no_argument, 0, DISNIX_OPTION_TRANSFER_ONLY},
        {"depth-first", no_argument, 0, DISNIX_OPTION_DEPTH_FIRST},
        {"no-upgrade", no_argument, 0, DISNIX_OPTION_NO_UPGRADE},
        {"all", no_argument, 0, DISNIX_OPTION_ALL},
        {"keep", required_argument, 0, DISNIX_OPTION_KEEP},
        {"max-concurrent-transfers", required_argument, 0, 'm'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };

    unsigned int max_concurrent_transfers = 2;
    unsigned int flags = 0;
    int keep = 1;
    char *old_manifest = NULL;
    char *profile = NULL;
    char *coordinator_profile_path = NULL;
    char *manifest_file;
    char *container = NULL;
    char *component = NULL;

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "c:C:m:o:p:hv", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case 'c':
                container = optarg;
                break;
            case 'C':
                component = optarg;
                break;
            case 'p':
                profile = optarg;
                break;
            case DISNIX_OPTION_COORDINATOR_PROFILE_PATH:
                coordinator_profile_path = optarg;
                break;
            case 'o':
                old_manifest = optarg;
                break;
            case DISNIX_OPTION_NO_UPGRADE:
                flags |= FLAG_NO_UPGRADE;
                break;
            case DISNIX_OPTION_ALL:
                flags |= FLAG_ALL;
                break;
            case DISNIX_OPTION_KEEP:
                keep = atoi(optarg);
                break;
            case DISNIX_OPTION_TRANSFER_ONLY:
                flags |= FLAG_TRANSFER_ONLY;
                break;
            case DISNIX_OPTION_DEPTH_FIRST:
                flags |= FLAG_DEPTH_FIRST;
                break;
            case 'm':
                max_concurrent_transfers = atoi(optarg);
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            case '?':
                print_usage(argv[0]);
                return 1;
            case 'v':
                print_version(argv[0]);
                return 0;
        }
    }

    /* Validate options */

    profile = check_profile_option(profile);

    if(optind >= argc)
        manifest_file = NULL;
    else
        manifest_file = argv[optind];

    return run_restore(manifest_file, max_concurrent_transfers, flags, keep, old_manifest, coordinator_profile_path, profile, container, component); /* Execute restore operation */
}
