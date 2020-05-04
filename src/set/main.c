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
#include "run-set-profiles.h"

static void print_usage(const char *command)
{
    printf("Usage: %s [OPTION] MANIFEST\n\n", command);

    puts(
    "The command `disnix-set' updates the coordinator profile referring to the last\n"
    "deployed manifest and the Disnix profiles on the target machines referring\n"
    "to the set of installed services. Updating the profiles prevents the\n"
    "configuration from being garbage collected.\n\n"

    "This command should almost never be called directly. The command `disnix-env'\n"
    "invokes this command to update the profiles automatically.\n\n"

    "Options:\n"
    "  -p, --profile=PROFILE                Name of the profile in which the services\n"
    "                                       are registered. Defaults to: default\n"
    "      --coordinator-profile-path=PATH  Path to the manifest of the previous\n"
    "                                       configuration. By default this tool will\n"
    "                                       use the manifest stored in the disnix\n"
    "                                       coordinator profile instead of the\n"
    "                                       specified one, which is usually sufficient\n"
    "                                       in most cases.\n"
    "      --no-coordinator-profile         Specifies that the coordinator profile\n"
    "                                       should not be updated\n"
    "      --no-target-profiles             Specifies that the target profiles should\n"
    "                                       not be updated\n"
    "  -h, --help                           Shows the usage of this command to the\n"
    "                                       user\n"
    "  -v, --version                        Shows the version of this command to the\n"
    "                                       user\n"
    );
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"profile", required_argument, 0, DISNIX_OPTION_PROFILE},
        {"coordinator-profile-path", required_argument, 0, DISNIX_OPTION_COORDINATOR_PROFILE_PATH},
        {"no-coordinator-profile", no_argument, 0, DISNIX_OPTION_NO_COORDINATOR_PROFILE},
        {"no-target-profiles", no_argument, 0, DISNIX_OPTION_NO_TARGET_PROFILES},
        {"help", no_argument, 0, DISNIX_OPTION_HELP},
        {"version", no_argument, 0, DISNIX_OPTION_VERSION},
        {0, 0, 0, 0}
    };
    char *profile = NULL;
    char *coordinator_profile_path = NULL;
    unsigned int flags = 0;

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "p:hv", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case DISNIX_OPTION_PROFILE:
                profile = optarg;
                break;
            case DISNIX_OPTION_COORDINATOR_PROFILE_PATH:
                coordinator_profile_path = optarg;
                break;
            case DISNIX_OPTION_NO_COORDINATOR_PROFILE:
                flags |= SET_NO_COORDINATOR_PROFILE;
                break;
            case DISNIX_OPTION_NO_TARGET_PROFILES:
                flags |= SET_NO_TARGET_PROFILES;
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

    profile = check_profile_option(profile);

    if(optind >= argc)
    {
        fprintf(stderr, "ERROR: No manifest specified!\n");
        return 1;
    }
    else
        return run_set_profiles(argv[optind], coordinator_profile_path, profile, flags); /* Execute set profiles operation */
}
