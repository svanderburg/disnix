/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2021  Sander van der Burg
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

#include "diagnose.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <defaultoptions.h>

static void print_usage(const char *command)
{
    printf("Usage: %s -S SERVICE [OPTION] [MANIFEST]\n\n", command);

    puts(
    "Spawns a remote shell session to a machine where the given service is deployed\n"
    "with environment variables containing configuration settings allowing a user to\n"
    "conveniently diagnose problems or execute arbitrary maintenance tasks.\n\n"

    "Options:\n"
    "  -S, --service              Name of the service to connect to\n"
    "      --show-mappings        Displays the targets and containers in which the\n"
    "                             service is hosted\n"
    "  -c, --container=CONTAINER  Name of the container in which the mutable\n"
    "                             component is deployed\n"
    "  -t, --target=TARGET        Specifies the target to connect to\n"
    "      --command=COMMAND      Shell commands to execute\n"
    "  -p, --profile=PROFILE      Name of the profile in which the services are\n"
    "                             registered. Defaults to: default\n"
    "      --coordinator-profile-path=PATH\n"
    "                             Path to the manifest of the previous configuration. By\n"
    "                             default this tool will use the manifest stored in the\n"
    "                             disnix coordinator profile instead of the specified\n"
    "                             one, which is usually sufficient in most cases.\n"
    "  -h, --help                 Shows the usage of this command to the user\n"
    "  -v, --version              Shows the version of this command to the user\n"

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
        {"service", required_argument, 0, DISNIX_OPTION_SERVICE},
        {"show-mappings", no_argument, 0, DISNIX_OPTION_SHOW_MAPPINGS},
        {"container", required_argument, 0, DISNIX_OPTION_CONTAINER},
        {"target", required_argument, 0, DISNIX_OPTION_TARGET},
        {"command", required_argument, 0, DISNIX_OPTION_COMMAND},
        {"coordinator-profile-path", required_argument, 0, DISNIX_OPTION_COORDINATOR_PROFILE_PATH},
        {"profile", required_argument, 0, DISNIX_OPTION_PROFILE},
        {"help", no_argument, 0, DISNIX_OPTION_HELP},
        {"version", no_argument, 0, DISNIX_OPTION_VERSION},
        {0, 0, 0, 0}
    };
    char *service_name = NULL;
    char *profile = NULL;
    char *coordinator_profile_path = NULL;
    char *manifest_file;
    char *container = NULL;
    char *target = NULL;
    char *command = NULL;
    int show_mappings = FALSE;

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "S:c:t:p:hv", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case DISNIX_OPTION_SERVICE:
                service_name = optarg;
                break;
            case DISNIX_OPTION_SHOW_MAPPINGS:
                show_mappings = TRUE;
                break;
            case DISNIX_OPTION_CONTAINER:
                container = optarg;
                break;
            case DISNIX_OPTION_TARGET:
                target = optarg;
                break;
            case DISNIX_OPTION_COMMAND:
                command = optarg;
                break;
            case DISNIX_OPTION_COORDINATOR_PROFILE_PATH:
                coordinator_profile_path = optarg;
                break;
            case DISNIX_OPTION_PROFILE:
                profile = optarg;
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

    if(service_name == NULL)
    {
        fprintf(stderr, "ERROR: No service name provided!\n");
        return 1;
    }

    if(optind >= argc)
        manifest_file = NULL;
    else
        manifest_file = argv[optind];

    return diagnose(service_name, show_mappings, manifest_file, coordinator_profile_path, profile, container, target, command);
}
