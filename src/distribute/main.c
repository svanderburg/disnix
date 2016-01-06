/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2016  Sander van der Burg
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
#include "distribute.h"

static void print_usage(const char *command)
{
    printf("Usage: %s [OPTION] MANIFEST\n\n", command);
    
    printf("The command `disnix-distribute' copies all the intra-dependency closures of\n");
    printf("services in a manifest file to the target machines in the network. This process\n");
    printf("is very efficient, since it scans for all intra-dependencies and only copies the\n");
    printf("missing parts.\n\n");

    printf("Most users don't need to use this command directly. The `disnix-env' command\n");
    printf("will automatically invoke this command to distribute the services if necessary.\n\n");
    
    printf("Options:\n");
    printf("  -m, --max-concurrent-transfers=NUM  Maximum amount of concurrent closure\n");
    printf("                                      transfers. Defauls to: 2\n");
    printf("  -h, --help                          Shows the usage of this command to the user\n");
    printf("  -v, --version                       Shows the version of this command to the\n");
    printf("                                      user\n");
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"max-concurrent-transfers", required_argument, 0, 'm'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };
    
    unsigned int max_concurrent_transfers = 2;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "m:hv", long_options, &option_index)) != -1)
    {
        switch(c)
        {
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
    
    if(optind >= argc)
    {
        fprintf(stderr, "ERROR: No manifest specified!\n");
        return 1;
    }
    else
        return distribute(argv[optind], max_concurrent_transfers); /* Execute distribute operation */
}
