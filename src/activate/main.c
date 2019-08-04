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
#include <getopt.h>
#include <defaultoptions.h>
#include "run-activate.h"

static void print_usage(const char *command)
{
    printf("Usage: %s [OPTION] MANIFEST\n\n", command);

    puts(
    "The command `disnix-activate' will activate all the services in the given\n"
    "manifest file on the target machines in the right order, by traversing the\n"
    "inter-dependency graph of the services.\n\n"
    "If there is already a configuration deployed, then this command will perform an\n"
    "upgrade.\n\n"

    "First it deactivates all obsolete services which are not present in the new\n"
    "configuration, finally it will activate all the new services in the new\n"
    "configuration. During this phase it takes inter-dependencies into account, so\n"
    "that no service will fail due to  a broken inter-dependency closure.\n\n"
    "In case of a failure, a rollback is performed and all the newly activated\n"
    "services are deactivated and all deactivated services are activated again.\n\n"

    "Most users don't need to use this command directly. The `disnix-env' command\n"
    "will automatically invoke this command to activate the new configuration.\n\n"

    "Options:\n"
    "  -p, --profile=PROFILE          Name of the profile in which the services are\n"
    "                                 registered. Defaults to: default\n"
    "      --coordinator-profile-path=PATH\n"
    "                                 Path to the manifest of the previous\n"
    "                                 configuration. By default this tool will use\n"
    "                                 the manifest stored in the disnix coordinator\n"
    "                                 profile instead of the specified one, which is\n"
    "                                 usually sufficient in most cases.\n"
    "  -o, --old-manifest=MANIFEST    Nix profile path where the manifest should be\n"
    "                                 stored, so that Disnix knows the current\n"
    "                                 configuration of a distributed system. By\n"
    "                                 default it is stored in the profile directory\n"
    "                                 of the user. Most users do not want to use this\n"
    "                                 option directly, but it is used by e.g. the\n"
    "                                 virtualization extension to store virtual\n"
    "                                 machine profile in a separate directory.\n"
    "      --no-upgrade               By enabling this option Disnix does not store\n"
    "                                 the deployment state for further use, such as\n"
    "                                 upgrading\n"
    "      --no-rollback              Do not roll back if an error occurs while\n"
    "                                 deactivating and activating services\n"
    "      --dry-run                  Prints the activation and deactivation steps\n"
    "                                 that will be performed but does not actually\n"
    "                                 execute them\n"
    "  -h, --help                     Shows the usage of this command to the user\n"
    "  -v, --version                  Shows the version of this command to the user\n"

    "\nExit status:\n"
    " 0                  Transition succeeded.\n"
    " 1                  Transition failed, but was successfully roll backed.\n"
    " 2                  Transition failed and the rollback of the obsolete mappings\n"
    "                    failed.\n"
    " 3                  Transition failed and the rollback of the new mappings\n"
    "                    failed.\n"

    "\nEnvironment:\n"
    "  DISNIX_PROFILE    Sets the name of the profile that stores the manifest on the\n"
    "                    coordinator machine and the deployed services per machine on\n"
    "                    each target (Defaults to: default)\n"
    );
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"old-manifest", required_argument, 0, DISNIX_OPTION_OLD_MANIFEST},
        {"coordinator-profile-path", required_argument, 0, DISNIX_OPTION_COORDINATOR_PROFILE_PATH},
        {"profile", required_argument, 0, DISNIX_OPTION_PROFILE},
        {"no-upgrade", no_argument, 0, DISNIX_OPTION_NO_UPGRADE},
        {"no-rollback", no_argument, 0, DISNIX_OPTION_NO_ROLLBACK},
        {"dry-run", no_argument, 0, DISNIX_OPTION_DRY_RUN},
        {"help", no_argument, 0, DISNIX_OPTION_HELP},
        {"version", no_argument, 0, DISNIX_OPTION_VERSION},
        {0, 0, 0, 0}
    };
    char *old_manifest = NULL;
    char *profile = NULL;
    char *coordinator_profile_path = NULL;
    unsigned int flags = 0;

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "o:p:hv", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case DISNIX_OPTION_OLD_MANIFEST:
                old_manifest = optarg;
                break;
            case DISNIX_OPTION_PROFILE:
                profile = optarg;
                break;
            case DISNIX_OPTION_COORDINATOR_PROFILE_PATH:
                coordinator_profile_path = optarg;
                break;
            case DISNIX_OPTION_NO_ROLLBACK:
                flags |= FLAG_NO_ROLLBACK;
                break;
            case DISNIX_OPTION_NO_UPGRADE:
                flags |= FLAG_NO_UPGRADE;
                break;
            case DISNIX_OPTION_DRY_RUN:
                flags |= FLAG_DRY_RUN;
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
        fprintf(stderr, "A manifest file has to be specified!\n");
        return 1;
    }
    else
        return run_activate_system(argv[optind], old_manifest, coordinator_profile_path, profile, flags); /* Execute activation operation */
}
