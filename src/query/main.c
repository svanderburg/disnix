/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2020  Sander van der Burg
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
#include <string.h>
#include <getopt.h>
#include <defaultoptions.h>
#include "query-installed.h"

static void print_usage(const char *command)
{
    printf("Usage: %s [OPTION] infrastructure_nix\n\n", command);

    puts(
    "The command `disnix-query' collects and displays all the installed services from\n"
    "the machines defined in a given infrastructure model.\n\n"

    "Options:\n"
    "  -p, --profile=PROFILE       Name of the profile in which the services are\n"
    "                              registered. Defaults to: default\n"
    "      --interface=INTERFACE   Path to executable that communicates with a Disnix\n"
    "                              interface. Defaults to `disnix-ssh-client'\n"
    "      --target-property=PROP  The target property of an infrastructure model,\n"
    "                              that specifies how to connect to the remote Disnix\n"
    "                              interface. (Defaults to hostname)\n"
    "  -f, --format=FORMAT         Output format. Options are: services (default),\n"
    "                              containers, nix, and xml\n"
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
    "  DISNIX_PROFILE             Sets the name of the profile that stores the\n"
    "                             manifest on the coordinator machine and the\n"
    "                             deployed services per machine on each target\n"
    "                             (Defaults to: default).\n"
    );
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"format", required_argument, 0, DISNIX_OPTION_FORMAT},
        {"interface", required_argument, 0, DISNIX_OPTION_INTERFACE},
        {"target-property", required_argument, 0, DISNIX_OPTION_TARGET_PROPERTY},
        {"profile", required_argument, 0, DISNIX_OPTION_PROFILE},
        {"xml", no_argument, 0, DISNIX_OPTION_XML},
        {"help", no_argument, 0, DISNIX_OPTION_HELP},
        {"version", no_argument, 0, DISNIX_OPTION_VERSION},
        {0, 0, 0, 0}
    };
    char *interface = NULL;
    char *target_property = NULL;
    char *profile = NULL;
    OutputFormat format = FORMAT_SERVICES;
    int xml = DISNIX_DEFAULT_XML;

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "f:p:hv", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case DISNIX_OPTION_FORMAT:
                if(strcmp(optarg, "services") == 0)
                    format = FORMAT_SERVICES;
                else if(strcmp(optarg, "containers") == 0)
                    format = FORMAT_CONTAINERS;
                else if(strcmp(optarg, "nix") == 0)
                    format = FORMAT_NIX;
                else if(strcmp(optarg, "xml") == 0)
                    format = FORMAT_XML;
                break;
            case DISNIX_OPTION_INTERFACE:
                interface = optarg;
                break;
            case DISNIX_OPTION_TARGET_PROPERTY:
                target_property = optarg;
                break;
            case DISNIX_OPTION_PROFILE:
                profile = optarg;
                break;
            case DISNIX_OPTION_XML:
                xml = TRUE;
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

    interface = check_interface_option(interface);
    target_property = check_target_property_option(target_property);
    profile = check_profile_option(profile);

    if(optind >= argc)
    {
        fprintf(stderr, "An infrastructure Nix expression has to be specified!\n");
        return 1;
    }
    else
        return query_installed(interface, target_property, argv[optind], profile, format, xml); /* Execute query operation */
}
