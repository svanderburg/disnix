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
#include "run-delete-state.h"

static void print_usage(const char *command)
{
    printf("Usage: %s [OPTION] [MANIFEST]\n\n", command);

    puts(
    "The command `disnix-delete-state' removes the state of all the components\n"
    "in a given deployment manifest that have been marked as garbage. If no manifest\n"
    "file is given, it uses the manifest of last deployed configuration.\n\n"

    "Most users don't need to use this command directly. The `disnix-env' command\n"
    "will automatically invoke this command after the new configuration has been\n"
    "deployed.\n\n"

    "Options:\n"
    "  -c, --container=CONTAINER      Name of the container in which the mutable\n"
    "                                 component is deployed\n"
    "  -C, --component=COMPONENT      Name of the mutable component to take snapshots\n"
    "                                 from\n"
    "  -p, --profile=PROFILE          Name of the profile in which the services are\n"
    "                                 registered. Defaults to: default\n"
    "      --coordinator-profile-path=PATH\n"
    "                                 Path to the manifest of the previous\n"
    "                                 configuration. By default this tool will use\n"
    "                                 the manifest stored in the disnix coordinator\n"
    "                                 profile instead of the specified one, which is\n"
    "                                 usually sufficient in most cases.\n"
    "  -h, --help                     Shows the usage of this command to the user\n"
    "  -v, --version                  Shows the version of this command to the user\n"

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
        {"coordinator-profile-path", required_argument, 0, 'P'},
        {"profile", required_argument, 0, 'p'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };
    
    char *manifest_file;
    char *profile = NULL;
    char *coordinator_profile_path = NULL;
    char *container = NULL;
    char *component = NULL;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "c:C:p:hv", long_options, &option_index)) != -1)
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
            case 'h':
            case '?':
                print_usage(argv[0]);
                return 0;
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
    
    return run_delete_state(manifest_file, coordinator_profile_path, profile, container, component); /* Execute snapshot operation */
}
