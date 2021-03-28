/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2021  Sander van der Burg
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
#include <glib.h>
#include <defaultoptions.h>
#include "run-activity.h"

static void print_usage(const char *command)
{
    printf("Usage: %s [OPTION] operation\n\n", command);

    puts(
    "The command `disnix-run-activity' provides direct access to any deployment\n"
    "activity Disnix carries out on a target machine.\n\n"

    "Options:\n\n"

    "Operations:\n"
    "      --import               Imports a given closure into the Nix store of the\n"
    "                             target machine\n"
    "      --export               Exports the closure of a given Nix store path of\n"
    "                             the target machine into a file\n"
    "      --print-invalid        Prints all the paths that are not valid in the Nix\n"
    "                             store of the target machine\n"
    "  -r, --realise              Realises the given store derivation on the target\n"
    "                             machine\n"
    "      --set                  Creates a Disnix profile only containing the given\n"
    "                             derivation on the target machine\n"
    "  -q, --query-installed      Queries all the installed services on the given\n"
    "                             target machine\n"
    "      --query-requisites     Queries all the requisites (intra-dependencies) of\n"
    "                             the given services on the target machine\n"
    "      --collect-garbage      Collects garbage on the given target machine\n"
    "      --activate             Activates the given service on the target machine\n"
    "      --deactivate           Deactivates the given service on the target machine\n"
    "      --lock                 Acquires a lock on a Disnix profile of the target\n"
    "                             machine\n"
    "      --unlock               Release the lock on a Disnix profile of the target\n"
    "                             machine\n"
    "      --snapshot             Snapshots the logical state of a component on the\n"
    "                             given target machine\n"
    "      --restore              Restores the logical state of a component on the\n"
    "                             given target machine\n"
    "      --delete-state         Deletes the state of a component on the given\n"
    "                             machine\n"
    "      --query-all-snapshots  Queries all available snapshots of a component on\n"
    "                             the given target machine\n"
    "      --query-latest-snapshot\n"
    "                             Queries the latest snapshot of a component on the\n"
    "                             given target machine\n"
    "      --print-missing-snapshots  Prints the paths of all snapshots not present on\n"
    "                             the given target machine\n"
    "      --import-snapshots     Imports the specified snapshots into the remote\n"
    "                             snapshot store\n"
    "      --export-snapshots     Exports the specified snapshot to the local\n"
    "                             snapshot store\n"
    "      --resolve-snapshots    Converts the relative paths to the snapshots to\n"
    "                             absolute paths\n"
    "      --clean-snapshots      Removes older snapshots from the snapshot store\n"
    "      --capture-config       Captures the configuration of the machine from the\n"
    "                             Dysnomia container properties in a Nix expression\n"
    "      --shell                Spawns a Dysnomia shell to run arbitrary\n"
    "                             maintenance tasks\n"
    "      --help                 Shows the usage of this command to the user\n"
    "      --version              Shows the version of this command to the user\n"

    "\nGeneral options:\n"
    "  -t, --target=TARGET        Specifies the target to connect to. This property\n"
    "                             is ignored by this client because it only supports\n"
    "                             loopback connections.\n"

    "\nImport/Export/Import snapshots/Export snapshots options:\n"
    "      --localfile            Specifies that the given paths are stored locally\n"
    "                             and must be transferred to the remote machine if\n"
    "                             needed\n"
    "      --remotefile           Specifies that the given paths are stored remotely\n"
    "                             and must transferred from the remote machine if\n"
    "                             needed\n"

    "\nSet/Query installed/Lock/Unlock options:\n"
    "  -p, --profile=PROFILE      Name of the Disnix profile. Defaults to: default\n"

    "\nCollect garbage options:\n"
    "  -d, --delete-old           Indicates whether all older generations of Nix\n"
    "                             profiles must be removed as well\n"

    "\nActivation/Deactivation/Snapshot/Restore/Delete state/Shell options:\n"
    "      --type=TYPE            Specifies the activation module that should be\n"
    "                             used, such as echo or process.\n"
    "      --arguments=ARGUMENTS  Specifies the arguments passed to the Dysnomia\n"
    "                             module, which is a string with key=value pairs\n"
    "      --container=CONTAINER  Name of the container in which the component is\n"
    "                             managed. If omitted it will default to the same\n"
    "                             value as the type.\n"

    "\nShell options:\n"
    "      --command=COMMAND      Commands to execute in the shell session\n"

    "\nQuery all snapshots/Query latest snapshot options:\n"
    "  -C, --container=CONTAINER  Name of the container in which the component is managed\n"
    "  -c, --component=COMPONENT  Name of the component hosted in a container\n"

    "\nClean snapshots options:\n"
    "      --keep=NUM             Amount of snapshot generations to keep. Defaults\n"
    "                             to: 1\n"
    "  -C, --container=CONTAINER  Name of the container to filter on\n"
    "  -c, --component=COMPONENT  Name of the component to filter on\n"

    "\nEnvironment:\n"
    "  DISNIX_PROFILE    Sets the name of the profile that stores the manifest on the\n"
    "                    coordinator machine and the deployed services per machine on\n"
    "                    each target (Defaults to: default).\n"
    );
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"import", no_argument, 0, 'I'},
        {"export", no_argument, 0, 'E'},
        {"print-invalid", no_argument, 0, 'P'},
        {"realise", no_argument, 0, 'r'},
        {"set", no_argument, 0, 'S'},
        {"query-installed", no_argument, 0, 'q'},
        {"query-requisites", no_argument, 0, 'Q'},
        {"collect-garbage", no_argument, 0, 'W'},
        {"activate", no_argument, 0, 'A'},
        {"deactivate", no_argument, 0, 'D'},
        {"delete-state", no_argument, 0, 'F'},
        {"lock", no_argument, 0, 'L'},
        {"unlock", no_argument, 0, 'U'},
        {"snapshot", no_argument, 0, 'f'},
        {"restore", no_argument, 0, 'g'},
        {"query-all-snapshots", no_argument, 0, 'B'},
        {"query-latest-snapshot", no_argument, 0, 's'},
        {"print-missing-snapshots", no_argument, 0, 'M'},
        {"import-snapshots", no_argument, 0, 'Y'},
        {"resolve-snapshots", no_argument, 0, 'Z'},
        {"clean-snapshots", no_argument, 0, 'e'},
        {"capture-config", no_argument, 0, '1'},
        {"shell", no_argument, 0, '2'},
        {"target", required_argument, 0, 't'},
        {"localfile", no_argument, 0, 'l'},
        {"remotefile", no_argument, 0, 'R'},
        {"profile", required_argument, 0, 'p'},
        {"delete-old", no_argument, 0, 'd'},
        {"type", required_argument, 0, 'T'},
        {"arguments", required_argument, 0, 'a'},
        {"container", required_argument, 0, 'C'},
        {"component", required_argument, 0, 'c'},
        {"keep", required_argument, 0, 'z'},
        {"command", required_argument, 0, '3'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };

    /* Option value declarations */
    Operation operation = OP_NONE;
    char *profile = NULL, *type = NULL, *container = NULL, *component = NULL, *command = NULL;
    gchar **derivation = NULL, **arguments = NULL;
    unsigned int derivation_size = 0, arguments_size = 0, flags = 0;
    int keep = 1;

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "rqt:p:dC:c:hv", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case 'I':
                operation = OP_IMPORT;
                break;
            case 'E':
                operation = OP_EXPORT;
                break;
            case 'P':
                operation = OP_PRINT_INVALID;
                break;
            case 'r':
                operation = OP_REALISE;
                break;
            case 'S':
                operation = OP_SET;
                break;
            case 'q':
                operation = OP_QUERY_INSTALLED;
                break;
            case 'Q':
                operation = OP_QUERY_REQUISITES;
                break;
            case 'W':
                operation = OP_COLLECT_GARBAGE;
                break;
            case 'A':
                operation = OP_ACTIVATE;
                break;
            case 'D':
                operation = OP_DEACTIVATE;
                break;
            case 'F':
                operation = OP_DELETE_STATE;
                break;
            case 'L':
                operation = OP_LOCK;
                break;
            case 'U':
                operation = OP_UNLOCK;
                break;
            case 'f':
                operation = OP_SNAPSHOT;
                break;
            case 'g':
                operation = OP_RESTORE;
                break;
            case 'B':
                operation = OP_QUERY_ALL_SNAPSHOTS;
                break;
            case 's':
                operation = OP_QUERY_LATEST_SNAPSHOT;
                break;
            case 'M':
                operation = OP_PRINT_MISSING_SNAPSHOTS;
                break;
            case 'Y':
                operation = OP_IMPORT_SNAPSHOTS;
                break;
            case 'Z':
                operation = OP_RESOLVE_SNAPSHOTS;
                break;
            case 'e':
                operation = OP_CLEAN_SNAPSHOTS;
                break;
            case '1':
                operation = OP_CAPTURE_CONFIG;
                break;
            case '2':
                operation = OP_SHELL;
                break;
            case 't':
                break;
            case 'l':
                break;
            case 'R':
                break;
            case 'p':
                profile = optarg;
                break;
            case 'd':
                flags |= FLAG_DELETE_OLD;
                break;
            case 'T':
                type = optarg;
                break;
            case 'a':
                arguments = (gchar**)g_realloc(arguments, (arguments_size + 1) * sizeof(gchar*));
                arguments[arguments_size] = g_strdup(optarg);
                arguments_size++;
                break;
            case 'C':
                container = optarg;
                break;
            case 'c':
                component = optarg;
                break;
            case 'z':
                keep = atoi(optarg);
                break;
            case '3':
                command = optarg;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            case '?':
                print_usage(argv[0]);
                return 1;
            case 'v':
                print_version(argv[0]);
                return 0;
        }
    }

    /* Validate options */
    profile = check_profile_option(profile);

    /* Validate non-options */
    while(optind < argc)
    {
        derivation = g_realloc(derivation, (derivation_size + 1) * sizeof(gchar*));
        derivation[derivation_size] = g_strdup(argv[optind]);
        derivation_size++;
        optind++;
    }

    derivation = g_realloc(derivation, (derivation_size + 1) * sizeof(gchar*));
    derivation[derivation_size] = NULL;

    /* Add NULL termination to the arguments vector */
    arguments = g_realloc(arguments, (arguments_size + 1) * sizeof(gchar*));
    arguments[arguments_size] = NULL;

    /* Execute Disnix activity */
    return run_disnix_activity(operation, derivation, flags, profile, arguments, type, container, component, keep, command);
}
