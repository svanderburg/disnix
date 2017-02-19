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
#include <glib.h>
#include <defaultoptions.h>
#include "disnix-client.h"

static void print_usage(const char *command)
{
    printf("Usage: %s [OPTION] operation\n\n", command);
    
    printf("The command `disnix-client' provides access to a `disnix-service' instance\n");
    printf("running on the same machine by connecting to the D-Bus system or session bus.\n\n");
    
    printf("In most cases this tool is only needed for debugging purposes, since it only uses\n");
    printf("the D-Bus protocol and cannot connect to a remote machine. A more useful client\n");
    printf("for use in production environments is: `disnix-ssh-client'.\n\n");
    
    printf("Options:\n\n");
    
    printf("Operations:\n");
    printf("  --import                   Imports a given closure into the Nix store of the\n");
    printf("                             target machine\n");
    printf("  --export                   Exports the closure of a given Nix store path of\n");
    printf("                             the target machine into a file\n");
    printf("  --print-invalid            Prints all the paths that are not valid in the Nix\n");
    printf("                             store of the target machine\n");
    printf("  -r, --realise              Realises the given store derivation on the target\n");
    printf("                             machine\n");
    printf("  --set                      Creates a Disnix profile only containing the given\n");
    printf("                             derivation on the target machine\n");
    printf("  -q, --query-installed      Queries all the installed services on the given\n");
    printf("                             target machine\n");
    printf("  --query-requisites         Queries all the requisites (intra-dependencies) of\n");
    printf("                             the given services on the target machine\n");
    printf("  --collect-garbage          Collects garbage on the given target machine\n");
    printf("  --activate                 Activates the given service on the target machine\n");
    printf("  --deactivate               Deactivates the given service on the target machine\n");
    printf("  --lock                     Acquires a lock on a Disnix profile of the target\n");
    printf("                             machine\n");
    printf("  --unlock                   Release the lock on a Disnix profile of the target\n");
    printf("                             machine\n");
    printf("  --snapshot                 Snapshots the logical state of a component on the\n");
    printf("                             given target machine\n");
    printf("  --restore                  Restores the logical state of a component on the\n");
    printf("                             given target machine\n");
    printf("  --delete-state             Deletes the state of a component on the given\n");
    printf("                             machine\n");
    printf("  --query-all-snapshots      Queries all available snapshots of a component on\n");
    printf("                             the given target machine\n");
    printf("  --query-latest-snapshot    Queries the latest snapshot of a component on the\n");
    printf("                             given target machine\n");
    printf("  --print-missing-snapshots  Prints the paths of all snapshots not present on\n");
    printf("                             the given target machine\n");
    printf("  --import-snapshots         Imports the specified snapshots into the remote\n");
    printf("                             snapshot store\n");
    printf("  --export-snapshots         Exports the specified snapshot to the local\n");
    printf("                             snapshot store\n");
    printf("  --resolve-snapshots        Converts the relative paths to the snapshots to\n");
    printf("                             absolute paths\n");
    printf("  --clean-snapshots          Removes older snapshots from the snapshot store\n");
    printf("  --capture-config           Captures the configuration of the machine from the\n");
    printf("                             Dysnomia container properties in a Nix expression\n");
    printf("  --help                     Shows the usage of this command to the user\n");
    printf("  --version                  Shows the version of this command to the user\n");

    printf("\nGeneral options:\n");
    printf("  -t, --target=TARGET        Specifies the target to connect to. This property\n");
    printf("                             is ignored by this client because it only supports\n");
    printf("                             loopback connections.\n");
    printf("  --session-bus              Connects to the session bus instead of the system\n");
    printf("                             bus. This is useful for testing/debugging purposes\n");
    
    printf("\nImport/Export/Import snapshots/Export snapshots options:\n");
    printf("  --localfile                Specifies that the given paths are stored locally\n");
    printf("                             and must be transferred to the remote machine if\n");
    printf("                             needed\n");
    printf("  --remotefile               Specifies that the given paths are stored remotely\n");
    printf("                             and must transferred from the remote machine if\n");
    printf("                             needed\n");
    
    printf("\nSet/Query installed/Lock/Unlock options:\n");
    printf("  -p, --profile=PROFILE      Name of the Disnix profile. Defaults to: default\n");
  
    printf("\nCollect garbage options:\n");
    printf("  -d, --delete-old           Indicates whether all older generations of Nix\n");
    printf("                             profiles must be removed as well\n");

    printf("\nActivation/Deactivation/Snapshot/Restore/Delete state options:\n");
    printf("  --type=TYPE                Specifies the activation module that should be\n");
    printf("                             used, such as echo or process.\n");
    printf("  --arguments=ARGUMENTS      Specifies the arguments passed to the Dysnomia\n");
    printf("                             module, which is a string with key=value pairs\n");
    printf("  --container=CONTAINER      Name of the container in which the component is\n");
    printf("                             managed. If omitted it will default to the same\n");
    printf("                             value as the type.\n");

    printf("\nQuery all snapshots/Query latest snapshot options:\n");
    printf("  -C, --container=CONTAINER  Name of the container in which the component is managed\n");
    printf("  -c, --component=COMPONENT  Name of the component hosted in a container\n");

    printf("\nClean snapshots options:\n");
    printf("  --keep=NUM                 Amount of snapshot generations to keep. Defaults\n");
    printf("                             to: 1\n");
    printf("  -C, --container=CONTAINER  Name of the container to filter on\n");
    printf("  -c, --component=COMPONENT  Name of the component to filter on\n");

    printf("\nEnvironment:\n");
    printf("  DISNIX_PROFILE    Sets the name of the profile that stores the manifest on the\n");
    printf("                    coordinator machine and the deployed services per machine on\n");
    printf("                    each target (Defaults to: default).\n");
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
        {"target", required_argument, 0, 't'},
        {"localfile", no_argument, 0, 'l'},
        {"remotefile", no_argument, 0, 'R'},
        {"profile", required_argument, 0, 'p'},
        {"delete-old", no_argument, 0, 'd'},
        {"type", required_argument, 0, 'T'},
        {"arguments", required_argument, 0, 'a'},
        {"container", required_argument, 0, 'C'},
        {"component", required_argument, 0, 'c'},
        {"session-bus", no_argument, 0, 'b'},
        {"keep", required_argument, 0, 'z'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
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
            case 'b':
                flags |= FLAG_SESSION_BUS;
                break;
            case 'z':
                keep = atoi(optarg);
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
