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
#include <getopt.h>
#include <defaultoptions.h>
#include "activate.h"

static void print_usage(const char *command)
{
    printf("Usage: %s [OPTION] MANIFEST\n\n", command);
    
    printf("The command `disnix-activate' will activate all the services in the given\n");
    printf("manifest file on the target machines in the right order, by traversing the\n");
    printf("inter-dependency graph of the services.\n\n");
    printf("If there is already a configuration deployed, then this command will perform an\n");
    printf("upgrade.\n\n");
    
    printf("First it deactivates all obsolete services which are not present in the new\n");
    printf("configuration, finally it will activate all the new services in the new\n");
    printf("configuration. During this phase it takes inter-dependencies into account, so\n");
    printf("that no service will fail due to  a broken inter-dependency closure.\n\n");
    printf("In case of a failure, a rollback is performed and all the newly activated\n");
    printf("services are deactivated and all deactivated services are activated again.\n\n");
    
    printf("Most users don't need to use this command directly. The `disnix-env' command\n");
    printf("will automatically invoke this command to activate the new configuration.\n\n");
    
    printf("Options:\n");
    printf("  -p, --profile=PROFILE          Name of the profile in which the services are\n");
    printf("                                 registered. Defaults to: default\n");
    printf("      --coordinator-profile-path=PATH\n");
    printf("                                 Path to the manifest of the previous\n");
    printf("                                 configuration. By default this tool will use\n");
    printf("                                 the manifest stored in the disnix coordinator\n");
    printf("                                 profile instead of the specified one, which is\n");
    printf("                                 usually sufficient in most cases.\n");
    printf("  -o, --old-manifest=MANIFEST    Nix profile path where the manifest should be\n");
    printf("                                 stored, so that Disnix knows the current\n");
    printf("                                 configuration of a distributed system. By\n");
    printf("                                 default it is stored in the profile directory\n");
    printf("                                 of the user. Most users do not want to use this\n");
    printf("                                 option directly, but it is used by e.g. the\n");
    printf("                                 virtualization extension to store virtual\n");
    printf("                                 machine profile in a separate directory.\n");
    printf("      --no-upgrade               By enabling this option Disnix does not store\n");
    printf("                                 the deployment state for further use, such as\n");
    printf("                                 upgrading\n");
    printf("      --dry-run                  Prints the activation and deactivation steps\n");
    printf("                                 that will be performed but does not actually\n");
    printf("                                 execute them\n");
    printf("  -h, --help                     Shows the usage of this command to the user\n");
    printf("  -v, --version                  Shows the version of this command to the user\n");
    
    printf("\nEnvironment:\n");
    printf("  DISNIX_PROFILE    Sets the name of the profile that stores the manifest on the\n");
    printf("                    coordinator machine and the deployed services per machine on\n");
    printf("                    each target (Defaults to: default)\n");
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"old-manifest", required_argument, 0, 'o'},
        {"coordinator-profile-path", required_argument, 0, 'P'},
        {"profile", required_argument, 0, 'p'},
        {"no-upgrade", no_argument, 0, 'u'},
        {"dry-run", no_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };
    char *old_manifest = NULL;
    char *profile = NULL;
    char *coordinator_profile_path = NULL;
    int no_upgrade = FALSE;
    int dry_run = FALSE;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "o:p:hv", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case 'o':
                old_manifest = optarg;
                break;
            case 'p':
                profile = optarg;
                break;
            case 'P':
                coordinator_profile_path = optarg;
                break;
            case 'u':
                no_upgrade = TRUE;
                break;
            case 'd':
                dry_run = TRUE;
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
        fprintf(stderr, "A manifest file has to be specified!\n");
        return 1;
    }
    else
        return activate_system(argv[optind], old_manifest, coordinator_profile_path, profile, no_upgrade, dry_run); /* Execute activation operation */
}
