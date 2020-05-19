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
#include <stdlib.h>
#include <getopt.h>
#include <defaultoptions.h>
#include <copy-snapshots.h>

static void print_usage(const char *command)
{
    printf(
    "Usage: %s [OPTION] --from --target TARGET -c CONTAINER -C COMPONENT\n"
    "  or: %s [OPTION] --to --target TARGET -c CONTAINER -C COMPONENT\n\n"
    , command, command);

    puts(
    "The command `disnix-copy-snapshots' transfers the logical state (typically\n"
    "represented as snapshots in a consistent and portable format) of a component\n"
    "residing in a container from and to a remote machine through a Disnix interface.\n\n"

    "Options:\n"
    "      --to                   Copy snapshots to the given target\n"
    "      --from                 Copy snapshots from the given target\n"
    "  -t, --target=TARGET        Address of the Disnix service running on the remote\n"
    "                             machine\n"
    "  -c, --container=CONTAINER  Name of the container in which the mutable\n"
    "                             component is deployed\n"
    "  -C, --component=COMPONENT  Name of the mutable component to take snapshots from\n"
    "      --all                  Transfer all snapshot generations instead of the\n"
    "                             latest only\n"
    "      --interface=INTERFACE  Path to executable that communicates with a Disnix\n"
    "                             interface. Defaults to: disnix-ssh-client\n"
    "  -h, --help                 Shows the usage of this command to the user\n"
    "  -v, --version              Shows the version of this command to the user\n\n"

    "Environment:\n"
    "  DISNIX_CLIENT_INTERFACE    Sets the client interface (defaults to:\n"
    "                             disnix-ssh-client)\n"
    "  DYSNOMIA_STATEDIR          Specifies where the snapshots must be stored on the\n"
    "                             coordinator machine (defaults to:\n"
    "                             /var/state/dysnomia)\n"
    );
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"from", no_argument, 0, 'F'},
        {"to", no_argument, 0, 'T'},
        {"target", required_argument, 0, 't'},
        {"container", required_argument, 0, DISNIX_OPTION_CONTAINER},
        {"component", required_argument, 0, DISNIX_OPTION_COMPONENT},
        {"all", no_argument, 0, DISNIX_OPTION_ALL},
        {"interface", required_argument, 0, DISNIX_OPTION_INTERFACE},
        {"help", no_argument, 0, DISNIX_OPTION_HELP},
        {"version", no_argument, 0, DISNIX_OPTION_VERSION},
        {0, 0, 0, 0}
    };
    int from = FALSE;
    int to = FALSE;
    char *interface = NULL;
    char *target = NULL;
    char *container = NULL;
    char *component = NULL;
    int all = FALSE;

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "t:C:c:hv", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case 'F':
                from = TRUE;
                break;
            case 'T':
                to = TRUE;
                break;
            case 't':
                target = optarg;
                break;
            case DISNIX_OPTION_CONTAINER:
                container = optarg;
                break;
            case DISNIX_OPTION_COMPONENT:
                component = optarg;
                break;
            case DISNIX_OPTION_ALL:
                all = TRUE;
                break;
            case DISNIX_OPTION_INTERFACE:
                interface = optarg;
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

    if(container == NULL)
    {
        fprintf(stderr, "ERROR: We need a container parameter!\n");
        return 1;
    }

    if(component == NULL)
    {
        fprintf(stderr, "ERROR: We need a component parameter!\n");
        return 1;
    }

    /* Execute operation */

    if(from && to)
    {
        fprintf(stderr, "ERROR: Either the --from or --to option must be used!\n");
        return 1;
    }
    else if(to)
        return !copy_snapshots_to_sync(interface, target, container, component, all);
    else if(from)
        return !copy_snapshots_from_sync(interface, target, container, component, all);
}
