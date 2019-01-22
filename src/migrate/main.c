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
#include "run-migrate.h"

static void print_usage(const char *command)
{
    printf("Usage: %s [OPTION] MANIFEST\n\n", command);

    puts(
    "The command `disnix-migrate' is used to snapshot, transfer and restore the\n"
    "state of services that have been moved from one machine to another.\n\n"

    "Most users don't need to use this command directly. The `disnix-env' command\n"
    "will automatically invoke this command to activate the new configuration.\n\n"

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
    "      --delete-state                   Remove the obsolete state of deactivated\n"
    "                                       services\n"
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
    "  DISNIX_PROFILE       Sets the name of the profile that stores the manifest on\n"
    "                       the coordinator machine and the deployed services per\n"
    "                       machine on each target (Defaults to: default)\n"
    "  DISNIX_DELETE_STATE  If set to 1 it automatically deletes the obsolete\n"
    "                       state after upgrading. (defaults to: 0)\n"
    "  DYSNOMIA_STATEDIR    Specifies where the snapshots must be stored on the\n"
    "                       coordinator machine (defaults to: /var/state/dysnomia)\n"
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
        {"coordinator-profile-path", required_argument, 0, 'P'},
        {"profile", required_argument, 0, 'p'},
        {"old-manifest", required_argument, 0, 'o'},
        {"no-upgrade", no_argument, 0, 'u'},
        {"delete-state", no_argument, 0, ','},
        {"transfer-only", no_argument, 0, 't'},
        {"depth-first", no_argument, 0, 'D'},
        {"all", no_argument, 0, 'a'},
        {"keep", required_argument, 0, 'k'},
        {"max-concurrent-transfers", required_argument, 0, 'm'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };

    unsigned int max_concurrent_transfers = 2;
    unsigned int flags = 0;
    int keep = 1;
    char *manifest_file;
    char *old_manifest = NULL;
    char *profile = NULL;
    char *coordinator_profile_path = NULL;
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
            case 'P':
                coordinator_profile_path = optarg;
                break;
            case 'o':
                old_manifest = optarg;
                break;
            case 'u':
                flags |= FLAG_NO_UPGRADE;
                break;
            case ',':
                flags |= FLAG_DELETE_STATE;
                break;
            case 'a':
                flags |= FLAG_ALL;
                break;
            case 'k':
                keep = atoi(optarg);
                break;
            case 't':
                flags |= FLAG_TRANSFER_ONLY;
                break;
            case 'D':
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

    if(check_global_delete_state())
        flags |= FLAG_DELETE_STATE;

    return run_migrate(manifest_file, max_concurrent_transfers, flags, keep, old_manifest, coordinator_profile_path, profile, container, component); /* Execute migrate operation */
}
