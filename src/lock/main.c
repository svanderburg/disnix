/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2015  Sander van der Burg
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
#include "locking.h"

static void print_usage(const char *command)
{
    printf("Usage: %s [--unlock] MANIFEST\n\n", command);
    
    printf("Notifies all services on the machines that the transition phase starts or ends,\n");
    printf("so that they can temporarily lock or unlock themselves (or take other\n");
    printf("precautions to make the transition to go smooth)\n\n");
    
    printf("Options:\n");
    printf("      --unlock           Executes an unlock operation instead of a lock\n");
    printf("  -p, --profile=PROFILE  Name of the profile in which the services are\n");
    printf("                         registered. Defaults to: default\n");
    printf("  -h, --help             Shows the usage of this command to the user\n");
    printf("  -v, --version          Shows the version of this command to the user\n");
    
    printf("\nEnvironment:\n");
    printf("  DISNIX_PROFILE    Sets the name of the profile that stores the manifest on the\n");
    printf("                    coordinator machine and the deployed services per machine on\n");
    printf("                    each target (Defaults to: default)\n");
}

static void print_version(const char *command)
{
    printf("%s (" PACKAGE_NAME ") " PACKAGE_VERSION "\n\n", command);
    printf("Copyright (C) 2008-2015 Sander van der Burg\n");
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"unlock", no_argument, 0, 'l'},
        {"profile", required_argument, 0, 'p'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };
    char *profile = NULL;
    int lock = TRUE;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "p:hv", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case 'l':
                lock = FALSE;
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
    
    if(optind >= argc)
    {
        fprintf(stderr, "ERROR: No manifest specified!\n");
        return 1;
    }
    else
        return lock_or_unlock(lock, argv[optind], profile); /* Execute lock or unlock operation */
}
