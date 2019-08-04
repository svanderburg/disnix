/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2019  Sander van der Burg
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
#include "run-distribute.h"

static void print_usage(const char *command)
{
    printf("Usage: %s [OPTION] MANIFEST\n\n", command);

    puts(
    "The command `disnix-distribute' copies all the intra-dependency closures of\n"
    "services in a manifest file to the target machines in the network. This process\n"
    "is very efficient, since it scans for all intra-dependencies and only copies the\n"
    "missing parts.\n\n"

    "Most users don't need to use this command directly. The `disnix-env' command\n"
    "will automatically invoke this command to distribute the services if necessary.\n\n"

    "Options:\n"
    "  -m, --max-concurrent-transfers=NUM  Maximum amount of concurrent closure\n"
    "                                      transfers. Defauls to: 2\n"
    "  -h, --help                          Shows the usage of this command to the user\n"
    "  -v, --version                       Shows the version of this command to the\n"
    "                                      user\n"
    );
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"max-concurrent-transfers", required_argument, 0, DISNIX_OPTION_MAX_CONCURRENT_TRANSFERS},
        {"help", no_argument, 0, DISNIX_OPTION_HELP},
        {"version", no_argument, 0, DISNIX_OPTION_VERSION},
        {0, 0, 0, 0}
    };

    unsigned int max_concurrent_transfers = DISNIX_DEFAULT_MAX_NUM_OF_CONCURRENT_TRANSFERS;

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "m:hv", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case DISNIX_OPTION_MAX_CONCURRENT_TRANSFERS:
                max_concurrent_transfers = atoi(optarg);
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

    if(optind >= argc)
    {
        fprintf(stderr, "ERROR: No manifest specified!\n");
        return 1;
    }
    else
        return run_distribute(argv[optind], max_concurrent_transfers); /* Execute distribute operation */
}
