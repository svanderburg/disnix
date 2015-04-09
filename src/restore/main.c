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
#include "restore.h"

static void print_usage(const char *command)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s manifest\n", command);
    fprintf(stderr, "\nOptions:\n");
    fprintf(stderr, "--old-manifest manifest\n");
    fprintf(stderr, "--no-upgrade\n");
    fprintf(stderr, "--transfer-only\n");
    fprintf(stderr, "--all\n");
    fprintf(stderr, "--profile profile\n");
    fprintf(stderr, "--coordinator-profile-path path\n");
    fprintf(stderr, "-m | --max-concurrent-transfers\n");
    fprintf(stderr, "-h | --help\n");
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"coordinator-profile-path", required_argument, 0, 'P'},
        {"profile", required_argument, 0, 'p'},
        {"old-manifest", required_argument, 0, 'o'},
        {"transfer-only", no_argument, 0, 't'},
        {"no-upgrade", no_argument, 0, 'u'},
        {"all", no_argument, 0, 'a'},
        {"max-concurrent-transfers", required_argument, 0, 'm'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    unsigned int max_concurrent_transfers = 2;
    int transfer_only = FALSE;
    int all = FALSE;
    int no_upgrade = FALSE;
    char *old_manifest = NULL;
    char *profile = NULL;
    char *coordinator_profile_path = NULL;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "m:p:h", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case 'p':
                profile = optarg;
                break;
            case 'P':
                coordinator_profile_path = optarg;
                break;
            case 'o':
                old_manifest = optarg;
                break;
            case 'u':
                no_upgrade = TRUE;
                break;
            case 'a':
                all = TRUE;
                break;
            case 't':
                transfer_only = TRUE;
                break;
            case 'm':
                max_concurrent_transfers = atoi(optarg);
                break;
            case 'h':
            case '?':
                print_usage(argv[0]);
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
        return restore(argv[optind], max_concurrent_transfers, transfer_only, all, old_manifest, coordinator_profile_path, profile, no_upgrade); /* Execute restore operation */
}
