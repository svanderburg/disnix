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
#include "capture-manifest.h"

static void print_usage(const char *command)
{
    printf("Usage: %s [OPTION] infrastructure_nix\n\n", command);
    
    printf("The command `disnix-capture-manifest' captures the manifests of the target\n");
    printf("Disnix profiles, retrieves their intra-dependency closures and composes a Nix\n");
    printf("expression that can be used to reconstruct the deployment manifest on the\n");
    printf("coordinator machine.\n\n");
    
    printf("Options:\n");
    printf("  -p, --profile=PROFILE       Name of the profile in which the services are\n");
    printf("                              registered. Defaults to: default\n");
    printf("      --interface=INTERFACE   Path to executable that communicates with a Disnix\n");
    printf("                              interface. Defaults to `disnix-ssh-client'\n");
    printf("      --target-property=PROP  The target property of an infrastructure model,\n");
    printf("                              that specifies how to connect to the remote Disnix\n");
    printf("                              interface. (Defaults to hostname)\n");
    printf("  -m, --max-concurrent-transfers=NUM\n");
    printf("                              Maximum amount of concurrent closure transfers.\n");
    printf("                              Defauls to: 2\n");
    printf("  -h, --help                  Shows the usage of this command to the user\n");
    printf("  -v, --version               Shows the version of this command to the user\n");
    
    printf("\nEnvironment:\n");
    printf("  DISNIX_CLIENT_INTERFACE    Sets the client interface (which defaults to\n");
    printf("                             `disnix-ssh-client')\n");
    printf("  DISNIX_TARGET_PROPERTY     Specifies which property in the infrastructure Nix\n");
    printf("                             expression specifies how to connect to the remote\n");
    printf("                             interface (defaults to: hostname)\n");
    printf("  DISNIX_PROFILE             Sets the name of the profile that stores the\n");
    printf("                             manifest on the coordinator machine and the\n");
    printf("                             deployed services per machine on each target\n");
    printf("                             (Defaults to: default).\n");
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"interface", required_argument, 0, 'i'},
        {"target-property", required_argument, 0, 't'},
        {"profile", required_argument, 0, 'p'},
        {"max-concurrent-transfers", required_argument, 0, 'm'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };
    char *interface = NULL;
    char *target_property = NULL;
    char *profile = NULL;
    unsigned int max_concurrent_transfers = 2;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "p:m:hv", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case 'i':
                interface = optarg;
                break;
            case 't':
                target_property = optarg;
                break;
            case 'p':
                profile = optarg;
                break;
            case 'm':
                max_concurrent_transfers = atoi(optarg);
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
    
    interface = check_interface_option(interface);
    target_property = check_target_property_option(target_property);
    profile = check_profile_option(profile);
    
    if(optind >= argc)
    {
        fprintf(stderr, "An infrastructure Nix expression has to be specified!\n");
        return 1;
    }
    else
        return capture_manifest(interface, target_property, argv[optind], profile, max_concurrent_transfers); /* Execute capture manifest operation */
}
