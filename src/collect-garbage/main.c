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

#include <stdio.h>
#include <getopt.h>
#include <defaultoptions.h>
#include "collect-garbage.h"

static void print_usage(const char *command)
{
    printf("Usage: %s [OPTION] infrastructure_nix\n\n", command);
    
    printf("The command `disnix-collect-garbage' collects all garbage from all the machines\n");
    printf("defined in an infrastructure Nix expression and optionally removes all the older\n");
    printf("profiles.\n\n");
    
    printf("Options:\n");
    printf("  -d, --delete-old            Removes all the old Nix profile generations\n");
    printf("      --interface=INTERFACE   Path to executable that communicates with a Disnix\n");
    printf("                              interface. Defaults to `disnix-ssh-client'\n");
    printf("      --target-property=PROP  The target property of an infrastructure model,\n");
    printf("                              that specifies how to connect to the remote Disnix\n");
    printf("                              interface. (Defaults to: hostname)\n");
    printf("  -h, --help                  Shows the usage of this command to the user\n");
    printf("  -v, --version               Shows the version of this command to the user\n");
    
    printf("\nEnvironment:\n");
    printf("  DISNIX_CLIENT_INTERFACE    Sets the client interface (which defaults to\n");
    printf("                             `disnix-ssh-client')\n");
    printf("  DISNIX_TARGET_PROPERTY     Specifies which property in the infrastructure Nix\n");
    printf("                             expression specifies how to connect to the remote\n");
    printf("                             interface (defaults to: hostname)\n");
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"interface", required_argument, 0, 'i'},
        {"target-property", required_argument, 0, 't'},
        {"delete-old", no_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };
    char *interface = NULL;
    gboolean delete_old = FALSE;
    char *target_property = NULL;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "dhv", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case 'i':
                interface = optarg;
                break;
            case 't':
                target_property = optarg;
                break;
            case 'd':
                delete_old = TRUE;
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
    
    if(optind >= argc)
    {
        fprintf(stderr, "An infrastructure Nix expression has to be specified!\n");
        return 1;
    }
    else
        return collect_garbage(interface, target_property, argv[optind], delete_old); /* Execute garbage collection operation */
}
