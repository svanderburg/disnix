/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2015  Sander van der Burg
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
#include "delete-state.h"

static void print_usage(const char *command)
{
    printf("Usage: %s [OPTION] [MANIFEST]\n\n", command);
    
    printf("The command `disnix-delete-state' removes all state of the components that are\n");
    printf("in the old deployment manifest, but not in the new deployment manifest. If no\n");
    printf("manifest file is given it uses the last deployed one.\n\n");
    
    printf("Most users don't need to use this command directly. The `disnix-env' command\n");
    printf("will automatically invoke this command after the new configuration has been\n");
    printf("deployed.\n\n");
    
    printf("Options:\n");
    printf("  -p, --profile=PROFILE          Name of the profile in which the services are\n");
    printf("                                 registered. Defaults to: default\n");
    printf("      --coordinator-profile-path=PATH\n");
    printf("                                 Path to the manifest of the previous\n");
    printf("                                 configuration. By default this tool will use\n");
    printf("                                 the manifest stored in the disnix coordinator\n");
    printf("                                 profile instead of the specified one, which is\n");
    printf("                                 usually sufficient in most cases.\n");
    printf("  -h, --help                     Shows the usage of this command to the user\n");
    printf("  -v, --version                  Shows the version of this command to the user\n");
    
    printf("\nEnvironment:\n");
    printf("  DISNIX_PROFILE    Sets the name of the profile that stores the manifest on the\n");
    printf("                    coordinator machine and the deployed services per machine on\n");
    printf("                    each target (Defaults to: default)\n");
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"coordinator-profile-path", required_argument, 0, 'P'},
        {"profile", required_argument, 0, 'p'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };
    
    char *manifest_file;
    char *profile = NULL;
    char *coordinator_profile_path = NULL;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "p:hv", long_options, &option_index)) != -1)
    {
        switch(c)
        {
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
    
    return delete_state(manifest_file, coordinator_profile_path, profile); /* Execute snapshot operation */
}
