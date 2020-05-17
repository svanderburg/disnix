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
#include <copy-closure.h>

static void print_usage(const char *command)
{
    printf(
    "Usage: %s [OPTION] --to --target TARGET paths\n"
    "  or: %s [OPTION] --from --target TARGET paths\n\n"
    , command, command);

    puts(
    "The command `disnix-copy-closure' copies a Nix store component and all its\n"
    "intra-dependencies to or from a given target machine through a Disnix\n"
    "interface. This process is very efficient, because it scans for all\n"
    "intra-dependencies and only copies the missing parts.\n\n"

    "This command is very similar to the `nix-copy-closure' command, except that it\n"
    "uses a Disnix interface for transport (which optionally uses SSH or a custom\n"
    "protocol) instead of using SSH directly.\n\n"

    "Options:\n"
    "      --to                   Copy closure to the given target\n"
    "      --from                 Copy closure from the given target\n"
    "  -t, --target=TARGET        Address of the Disnix service running on the remote\n"
    "                             machine\n"
    "      --interface=INTERFACE  Path to executable that communicates with a Disnix\n"
    "                             interface. Defaults to: disnix-ssh-client\n"
    "  -h, --help                 Shows the usage of this command to the user\n"
    "  -v, --version              Shows the version of this command to the user\n\n"

    "Environment:\n"
    "  DISNIX_CLIENT_INTERFACE    Sets the client interface (which defaults to:\n"
    "                             disnix-ssh-client)\n"
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
        {"interface", required_argument, 0, DISNIX_OPTION_INTERFACE},
        {"help", no_argument, 0, DISNIX_OPTION_HELP},
        {"version", no_argument, 0, DISNIX_OPTION_VERSION},
        {0, 0, 0, 0}
    };
    int from = FALSE;
    int to = FALSE;
    char *interface = NULL;
    char *target = NULL;
    char *tmpdir = NULL;
    char **derivations;

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "t:hv", long_options, &option_index)) != -1)
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

    if(tmpdir == NULL)
    {
        tmpdir = getenv("TMPDIR");

        if(tmpdir == NULL)
            tmpdir = "/tmp";
    }

    if(optind >= argc)
    {
        fprintf(stderr, "At least one path to the Nix store must be specified!\n");
        return 1;
    }
    else
        derivations = argv + optind;

    /* Execute operation */

    if(from && to)
    {
        fprintf(stderr, "ERROR: Either the --from or --to option must be used!\n");
        return 1;
    }
    else if(to)
        return !copy_closure_to_sync(interface, target, tmpdir, derivations);
    else if(from)
    {
        int result = copy_closure_from_sync(interface, target, derivations);
        fprintf(stderr, "THE RESULT IS: %d\n", result);
        return !result;
    }
}
