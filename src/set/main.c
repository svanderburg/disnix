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
#include "profiles.h"

static void print_usage(const char *command)
{
    printf("Usage: %s [OPTION] MANIFEST\n\n", command);
    
    printf("The command `disnix-set' updates the coordinator profile referring to the last\n");
    printf("deployed manifest and the Disnix profiles on the target machines referring\n");
    printf("to the set of installed services. Updating the profiles prevents the\n");
    printf("configuration from being garbage collected.\n\n");
    
    printf("This command should almost never be called directly. The command `disnix-env'\n");
    printf("invokes this command to update the profiles automatically.\n\n");
    
    printf("Options:\n");
    printf("  -p, --profile=PROFILE                Name of the profile in which the services\n");
    printf("                                       are registered. Defaults to: default\n");
    printf("      --coordinator-profile-path=PATH  Path to the manifest of the previous\n");
    printf("                                       configuration. By default this tool will\n");
    printf("                                       use the manifest stored in the disnix\n");
    printf("                                       coordinator profile instead of the\n");
    printf("                                       specified one, which is usually sufficient\n");
    printf("                                       in most cases.\n");
    printf("      --no-coordinator-profile         Specifies that the coordinator profile\n");
    printf("                                       should not be updated\n");
    printf("      --no-target-profiles             Specifies that the target profiles should\n");
    printf("                                       not be updated\n");
    printf("  -h, --help                           Shows the usage of this command to the\n");
    printf("                                       user\n");
    printf("  -v, --version                        Shows the version of this command to the\n");
    printf("                                       user\n");
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"profile", required_argument, 0, 'p'},
        {"coordinator-profile-path", required_argument, 0, 'P'},
        {"no-coordinator-profile", no_argument, 0, 'c'},
        {"no-target-profiles", no_argument, 0, 'C'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };
    char *profile = NULL;
    char *coordinator_profile_path = NULL;
    int no_coordinator_profile = FALSE;
    int no_target_profiles = FALSE;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "p:hv", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case 'p':
                profile = optarg;
                break;
            case 'P':
                coordinator_profile_path = optarg;
                break;
            case 'c':
                no_coordinator_profile = TRUE;
                break;
           case 'C':
                no_target_profiles = TRUE;
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
        return set_profiles(argv[optind], coordinator_profile_path, profile, no_coordinator_profile, no_target_profiles); /* Execute set profiles operation */
}
