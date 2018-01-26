/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2017  Sander van der Burg
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

    printf("Spawns a remote shell session to a machine where the given service is deployed\n");
    printf("with environment variables containing configuration settings allowing a user to\n");
    printf("conveniently diagnose problems or execute arbitrary maintenance tasks.\n\n");

    printf("Options:\n");
    printf("  -S, --service              Name of the service to connect to\n");
    printf("  -c, --container=CONTAINER  Name of the container in which the mutable\n");
    printf("                             component is deployed\n");
    printf("  -t, --target=TARGET        Specifies the target to connect to\n");
    printf("      --command=COMMAND      Shell commands to execute\n");
    printf("  -p, --profile=PROFILE      Name of the profile in which the services are\n");
    printf("                             registered. Defaults to: default\n");
    printf("      --coordinator-profile-path=PATH\n");
    printf("                             Path to the manifest of the previous configuration. By\n");
    printf("                             default this tool will use the manifest stored in the\n");
    printf("                             disnix coordinator profile instead of the specified\n");
    printf("                             one, which is usually sufficient in most cases.\n");
    printf("  -h, --help                 Shows the usage of this command to the user\n");
    printf("  -v, --version              Shows the version of this command to the user\n");

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
        {"service", required_argument, 0, 'S'},
        {"container", required_argument, 0, 'c'},
        {"target", required_argument, 0, 't'},
        {"command", required_argument, 0, 'C'},
        {"coordinator-profile-path", required_argument, 0, 'P'},
        {"profile", required_argument, 0, 'p'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };
    char *service_name = NULL;
    char *profile = NULL;
    char *coordinator_profile_path = NULL;
    char *manifest_file;
    char *container = NULL;
    char *target = NULL;
    char *command = NULL;

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "S:c:t:p:hv", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case 'S':
                service_name = optarg;
                break;
            case 'c':
                container = optarg;
                break;
            case 't':
                target = optarg;
                break;
            case 'C':
                command = optarg;
                break;
            case 'P':
                coordinator_profile_path = optarg;
                break;
            case 'p':
                profile = optarg;
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

    if(service_name == NULL)
    {
        fprintf(stderr, "ERROR: No service name provided!\n");
        return 1;
    }

    if(optind >= argc)
        manifest_file = NULL;
    else
        manifest_file = argv[optind];

    return diagnose(service_name, manifest_file, coordinator_profile_path, profile, container, target, command);
}
