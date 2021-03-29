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
#include "disnix-client.h"

static void print_usage(const char *command)
{
    printf("Usage: %s [OPTION] operation\n\n", command);

    puts(
    "The command `disnix-client' provides access to a `disnix-service' instance\n"
    "running on the same machine by connecting to the D-Bus system or session bus.\n\n"

    "In most cases this tool is only needed for debugging purposes, since it only uses\n"
    "the D-Bus protocol and cannot connect to a remote machine. A more useful client\n"
    "for use in production environments is: `disnix-ssh-client'.\n\n"

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
    "      --print-missing-snapshots\n"
    "                             Prints the paths of all snapshots not present on\n"
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
    "      --help                 Shows the usage of this command to the user\n"
    "      --version              Shows the version of this command to the user\n"

    "\nGeneral options:\n"
    "  -t, --target=TARGET        Specifies the target to connect to. This property\n"
    "                             is ignored by this client because it only supports\n"
    "                             loopback connections.\n"
    "      --session-bus          Connects to the session bus instead of the system\n"
    "                             bus. This is useful for testing/debugging purposes\n"

    "\nImport/Export/Import snapshots/Export snapshots options:\n"
    "      --localfile            Specifies that the given paths are stored locally\n"
    "                             and must be transferred to the remote machine if\n"
    "                             needed\n"
    "      --remotefile           Specifies that the given paths are stored remotely\n"
    "                             and must transferred from the remote machine if\n"
    "                             needed\n"

    "\nShell options:\n"
    "      --command=COMMAND      Commands to execute in the shell session\n"

    "\nSet/Query installed/Lock/Unlock options:\n"
    "  -p, --profile=PROFILE      Name of the Disnix profile. Defaults to: default\n"

    "\nCollect garbage options:\n"
    "  -d, --delete-old           Indicates whether all older generations of Nix\n"
    "                             profiles must be removed as well\n"

    "\nActivation/Deactivation/Snapshot/Restore/Delete state options:\n"
    "      --type=TYPE            Specifies the activation module that should be\n"
    "                             used, such as echo or process.\n"
    "      --arguments=ARGUMENTS  Specifies the arguments passed to the Dysnomia\n"
    "                             module, which is a string with key=value pairs\n"
    "      --container=CONTAINER  Name of the container in which the component is\n"
    "                             managed. If omitted it will default to the same\n"
    "                             value as the type.\n"

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

typedef enum
{
    /* Operations */
    DISNIX_CLIENT_OPTION_IMPORT = 256,
    DISNIX_CLIENT_OPTION_EXPORT = 257,
    DISNIX_CLIENT_OPTION_PRINT_INVALID = 258,
    DISNIX_CLIENT_OPTION_REALISE = 'r',
    DISNIX_CLIENT_OPTION_SET = 259,
    DISNIX_CLIENT_OPTION_QUERY_INSTALLED = 'q',
    DISNIX_CLIENT_OPTION_QUERY_REQUISITES = 260,
    DISNIX_CLIENT_OPTION_COLLECT_GARBAGE = 261,
    DISNIX_CLIENT_OPTION_ACTIVATE = 262,
    DISNIX_CLIENT_OPTION_DEACTIVATE = 263,
    DISNIX_CLIENT_OPTION_DELETE_STATE = 264,
    DISNIX_CLIENT_OPTION_LOCK = 265,
    DISNIX_CLIENT_OPTION_UNLOCK = 266,
    DISNIX_CLIENT_OPTION_SNAPSHOT = 267,
    DISNIX_CLIENT_OPTION_RESTORE = 268,
    DISNIX_CLIENT_OPTION_QUERY_ALL_SNAPSHOTS = 269,
    DISNIX_CLIENT_OPTION_QUERY_LATEST_SNAPSHOT = 270,
    DISNIX_CLIENT_OPTION_PRINT_MISSING_SNAPSHOTS = 271,
    DISNIX_CLIENT_OPTION_IMPORT_SNAPSHOTS = 272,
    DISNIX_CLIENT_OPTION_RESOLVE_SNAPSHOTS = 273,
    DISNIX_CLIENT_OPTION_CLEAN_SNAPSHOTS = 274,
    DISNIX_CLIENT_OPTION_CAPTURE_CONFIG = 275,
    DISNIX_CLIENT_OPTION_SHELL = 276,
    DISNIX_CLIENT_OPTION_HELP = 'h',
    DISNIX_CLIENT_OPTION_VERSION = 'v',

    /* Options */
    DISNIX_CLIENT_OPTION_TARGET = 't',
    DISNIX_CLIENT_OPTION_LOCALFILE = 277,
    DISNIX_CLIENT_OPTION_REMOTEFILE = 278,
    DISNIX_CLIENT_OPTION_PROFILE = 'p',
    DISNIX_CLIENT_OPTION_DELETE_OLD = 'd',
    DISNIX_CLIENT_OPTION_TYPE = 279,
    DISNIX_CLIENT_OPTION_ARGUMENTS = 280,
    DISNIX_CLIENT_OPTION_CONTAINER = 'C',
    DISNIX_CLIENT_OPTION_COMPONENT = 'c',
    DISNIX_CLIENT_OPTION_KEEP = 281,
    DISNIX_CLIENT_OPTION_COMMAND = 282,
    DISNIX_CLIENT_OPTION_SESSION_BUS = 283
}
DisnixClientCommandLineOption;

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"import", no_argument, 0, DISNIX_CLIENT_OPTION_IMPORT},
        {"export", no_argument, 0, DISNIX_CLIENT_OPTION_EXPORT},
        {"print-invalid", no_argument, 0, DISNIX_CLIENT_OPTION_PRINT_INVALID},
        {"realise", no_argument, 0, DISNIX_CLIENT_OPTION_REALISE},
        {"set", no_argument, 0, DISNIX_CLIENT_OPTION_SET},
        {"query-installed", no_argument, 0, DISNIX_CLIENT_OPTION_QUERY_INSTALLED},
        {"query-requisites", no_argument, 0, DISNIX_CLIENT_OPTION_QUERY_REQUISITES},
        {"collect-garbage", no_argument, 0, DISNIX_CLIENT_OPTION_COLLECT_GARBAGE},
        {"activate", no_argument, 0, DISNIX_CLIENT_OPTION_ACTIVATE},
        {"deactivate", no_argument, 0, DISNIX_CLIENT_OPTION_DEACTIVATE},
        {"delete-state", no_argument, 0, DISNIX_CLIENT_OPTION_DELETE_STATE},
        {"lock", no_argument, 0, DISNIX_CLIENT_OPTION_LOCK},
        {"unlock", no_argument, 0, DISNIX_CLIENT_OPTION_UNLOCK},
        {"snapshot", no_argument, 0, DISNIX_CLIENT_OPTION_SNAPSHOT},
        {"restore", no_argument, 0, DISNIX_CLIENT_OPTION_RESTORE},
        {"query-all-snapshots", no_argument, 0, DISNIX_CLIENT_OPTION_QUERY_ALL_SNAPSHOTS},
        {"query-latest-snapshot", no_argument, 0, DISNIX_CLIENT_OPTION_QUERY_LATEST_SNAPSHOT},
        {"print-missing-snapshots", no_argument, 0, DISNIX_CLIENT_OPTION_PRINT_MISSING_SNAPSHOTS},
        {"import-snapshots", no_argument, 0, DISNIX_CLIENT_OPTION_IMPORT_SNAPSHOTS},
        {"resolve-snapshots", no_argument, 0, DISNIX_CLIENT_OPTION_RESOLVE_SNAPSHOTS},
        {"clean-snapshots", no_argument, 0, DISNIX_CLIENT_OPTION_CLEAN_SNAPSHOTS},
        {"capture-config", no_argument, 0, DISNIX_CLIENT_OPTION_CAPTURE_CONFIG},
        {"shell", no_argument, 0, DISNIX_CLIENT_OPTION_SHELL},
        {"target", required_argument, 0, DISNIX_CLIENT_OPTION_TARGET},
        {"localfile", no_argument, 0, DISNIX_CLIENT_OPTION_LOCALFILE},
        {"remotefile", no_argument, 0, DISNIX_CLIENT_OPTION_REMOTEFILE},
        {"profile", required_argument, 0, DISNIX_CLIENT_OPTION_PROFILE},
        {"delete-old", no_argument, 0, DISNIX_CLIENT_OPTION_DELETE_OLD},
        {"type", required_argument, 0, DISNIX_CLIENT_OPTION_TYPE},
        {"arguments", required_argument, 0, DISNIX_CLIENT_OPTION_ARGUMENTS},
        {"container", required_argument, 0, DISNIX_CLIENT_OPTION_CONTAINER},
        {"component", required_argument, 0, DISNIX_CLIENT_OPTION_COMPONENT},
        {"keep", required_argument, 0, DISNIX_CLIENT_OPTION_KEEP},
        {"command", required_argument, 0, DISNIX_CLIENT_OPTION_COMMAND},
        {"session-bus", no_argument, 0, DISNIX_CLIENT_OPTION_SESSION_BUS},
        {"help", no_argument, 0, DISNIX_CLIENT_OPTION_HELP},
        {"version", no_argument, 0, DISNIX_CLIENT_OPTION_VERSION},
        {0, 0, 0, 0}
    };

    /* Option value declarations */
    Operation operation = OP_NONE;
    char *profile = NULL, *type = NULL, *container = NULL, *component = NULL;
    gchar **derivation = NULL, **arguments = NULL;
    unsigned int derivation_size = 0, arguments_size = 0, flags = 0;
    int keep = 1;

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "rqt:p:dC:c:hv", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case DISNIX_CLIENT_OPTION_IMPORT:
                operation = OP_IMPORT;
                break;
            case DISNIX_CLIENT_OPTION_EXPORT:
                operation = OP_EXPORT;
                break;
            case DISNIX_CLIENT_OPTION_PRINT_INVALID:
                operation = OP_PRINT_INVALID;
                break;
            case DISNIX_CLIENT_OPTION_REALISE:
                operation = OP_REALISE;
                break;
            case DISNIX_CLIENT_OPTION_SET:
                operation = OP_SET;
                break;
            case DISNIX_CLIENT_OPTION_QUERY_INSTALLED:
                operation = OP_QUERY_INSTALLED;
                break;
            case DISNIX_CLIENT_OPTION_QUERY_REQUISITES:
                operation = OP_QUERY_REQUISITES;
                break;
            case DISNIX_CLIENT_OPTION_COLLECT_GARBAGE:
                operation = OP_COLLECT_GARBAGE;
                break;
            case DISNIX_CLIENT_OPTION_ACTIVATE:
                operation = OP_ACTIVATE;
                break;
            case DISNIX_CLIENT_OPTION_DEACTIVATE:
                operation = OP_DEACTIVATE;
                break;
            case DISNIX_CLIENT_OPTION_DELETE_STATE:
                operation = OP_DELETE_STATE;
                break;
            case DISNIX_CLIENT_OPTION_LOCK:
                operation = OP_LOCK;
                break;
            case DISNIX_CLIENT_OPTION_UNLOCK:
                operation = OP_UNLOCK;
                break;
            case DISNIX_CLIENT_OPTION_SNAPSHOT:
                operation = OP_SNAPSHOT;
                break;
            case DISNIX_CLIENT_OPTION_RESTORE:
                operation = OP_RESTORE;
                break;
            case DISNIX_CLIENT_OPTION_QUERY_ALL_SNAPSHOTS:
                operation = OP_QUERY_ALL_SNAPSHOTS;
                break;
            case DISNIX_CLIENT_OPTION_QUERY_LATEST_SNAPSHOT:
                operation = OP_QUERY_LATEST_SNAPSHOT;
                break;
            case DISNIX_CLIENT_OPTION_PRINT_MISSING_SNAPSHOTS:
                operation = OP_PRINT_MISSING_SNAPSHOTS;
                break;
            case DISNIX_CLIENT_OPTION_IMPORT_SNAPSHOTS:
                operation = OP_IMPORT_SNAPSHOTS;
                break;
            case DISNIX_CLIENT_OPTION_RESOLVE_SNAPSHOTS:
                operation = OP_RESOLVE_SNAPSHOTS;
                break;
            case DISNIX_CLIENT_OPTION_CLEAN_SNAPSHOTS:
                operation = OP_CLEAN_SNAPSHOTS;
                break;
            case DISNIX_CLIENT_OPTION_CAPTURE_CONFIG:
                operation = OP_CAPTURE_CONFIG;
                break;
            case DISNIX_CLIENT_OPTION_SHELL:
                operation = OP_SHELL;
                break;
            case DISNIX_CLIENT_OPTION_TARGET:
                break;
            case DISNIX_CLIENT_OPTION_LOCALFILE:
                break;
            case DISNIX_CLIENT_OPTION_REMOTEFILE:
                break;
            case DISNIX_CLIENT_OPTION_PROFILE:
                profile = optarg;
                break;
            case DISNIX_CLIENT_OPTION_DELETE_OLD:
                flags |= FLAG_DELETE_OLD;
                break;
            case DISNIX_CLIENT_OPTION_TYPE:
                type = optarg;
                break;
            case DISNIX_CLIENT_OPTION_ARGUMENTS:
                arguments = (gchar**)g_realloc(arguments, (arguments_size + 1) * sizeof(gchar*));
                arguments[arguments_size] = g_strdup(optarg);
                arguments_size++;
                break;
            case DISNIX_CLIENT_OPTION_CONTAINER:
                container = optarg;
                break;
            case DISNIX_CLIENT_OPTION_COMPONENT:
                component = optarg;
                break;
            case DISNIX_CLIENT_OPTION_KEEP:
                keep = atoi(optarg);
                break;
            case DISNIX_CLIENT_OPTION_SESSION_BUS:
                flags |= FLAG_SESSION_BUS;
                break;
            case DISNIX_CLIENT_OPTION_HELP:
                print_usage(argv[0]);
                return 0;
            case DISNIX_CLIENT_OPTION_VERSION:
                print_version(argv[0]);
                return 0;
            case '?':
                print_usage(argv[0]);
                return 1;
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

    /* Execute Disnix client */
    return run_disnix_client(operation, derivation, flags, profile, arguments, type, container, component, keep);
}
