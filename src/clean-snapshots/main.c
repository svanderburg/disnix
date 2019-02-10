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
#include "clean-snapshots.h"

static void print_usage(const char *command)
{
    printf("Usage: %s [OPTION] infrastructure_nix\n\n", command);

    puts(
    "The command `disnix-clean-snapshots' removes all older snapshot generations\n"
    "stored on the machines in the network.\n\n"

    "Options:\n"
    "      --interface=INTERFACE   Path to executable that communicates with a Disnix\n"
    "                              interface. Defaults to `disnix-ssh-client'\n"
    "      --target-property=PROP  The target property of an infrastructure model,\n"
    "                              that specifies how to connect to the remote Disnix\n"
    "      --keep=NUM              Amount of snapshot generations to keep. Defaults\n"
    "                              to: 1\n"
    "  -C, --container=CONTAINER   Name of the container to filter on\n"
    "  -c, --component=COMPONENT   Name of the component to filter on\n"
    "      --xml                   Specifies that the configurations are in XML not\n"
    "                              the Nix expression language.\n"
    "  -h, --help                  Shows the usage of this command to the user\n"
    "  -v, --version               Shows the version of this command to the user\n"

    "\nEnvironment:\n"
    "  DISNIX_CLIENT_INTERFACE    Sets the client interface (which defaults to\n"
    "                             `disnix-ssh-client')\n"
    "  DISNIX_TARGET_PROPERTY     Specifies which property in the infrastructure Nix\n"
    "                             expression specifies how to connect to the remote\n"
    "                             interface (defaults to: hostname)\n"
    );
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"interface", required_argument, 0, 'i'},
        {"target-property", required_argument, 0, 't'},
        {"keep", required_argument, 0, 'z'},
        {"container", required_argument, 0, 'C'},
        {"component", required_argument, 0, 'c'},
        {"xml", no_argument, 0, 'x'},
        {"version", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    char *interface = NULL;
    char *target_property = NULL;
    int keep = 1;
    char *container = NULL;
    char *component = NULL;
    int xml = FALSE;

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "c:C:hv", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case 'i':
                interface = optarg;
                break;
            case 't':
                target_property = optarg;
                break;
            case 'z':
                keep = atoi(optarg);
                break;
            case 'C':
                container = optarg;
                break;
            case 'c':
                component = optarg;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            case 'x':
                xml = TRUE;
                break;
            case '?':
                print_usage(argv[0]);
                return 1;
            case 'v':
                print_version(argv[0]);
                return 0;
        }
    }
    
    /* Validate options */
    
    interface = check_interface_option(interface);
    target_property = check_target_property_option(target_property);
    
    if(optind >= argc)
    {
        fprintf(stderr, "An infrastructure Nix expression has to be specified!\n");
        return 1;
    }
    else
        return clean_snapshots(interface, target_property, argv[optind], keep, container, component, xml); /* Execute clean snapshots operation */
}
