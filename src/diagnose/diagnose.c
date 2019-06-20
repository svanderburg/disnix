/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2018  Sander van der Burg
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

#include "diagnose.h"
#include <manifest.h>
#include <activationmapping.h>
#include <targets.h>
#include <client-interface.h>
#include <procreact_pid.h>

static int spawn_shell(GHashTable *targets_table, const ActivationMapping *mapping, char *command)
{
    Target *target = g_hash_table_lookup(targets_table, (gchar*)mapping->target);
    gchar **arguments = generate_activation_arguments(target, (gchar*)mapping->container);
    ProcReact_Status status;
    int exit_status;

    g_printerr("[%s]: Connecting to service: %s deployed to container: %s\n", mapping->target, mapping->service, mapping->container);

    exit_status = procreact_wait_for_exit_status(exec_dysnomia_shell((char*)target->client_interface, (char*)mapping->target, (char*)mapping->container, (char*)mapping->type, arguments, g_strv_length(arguments), (char*)mapping->service, command), &status);

    g_strfreev(arguments);

    return exit_status;
}

static void print_mappings(const GPtrArray *mappings_array)
{
    unsigned int i;

    g_printerr("This service has been mapped to:\n\n");

    for(i = 0; i < mappings_array->len; i++)
    {
        ActivationMapping *mapping = (ActivationMapping*)g_ptr_array_index(mappings_array, i);
        g_printerr("container: %s, target: %s\n", mapping->container, mapping->target);
    }
}

int diagnose(const char *service_name, const int show_mappings, const char *manifest_file, const gchar *coordinator_profile_path, gchar *profile, const char *container_filter, const char *target_filter, char *command)
{
    int exit_status;
    Manifest *manifest = open_provided_or_previous_manifest_file(manifest_file, coordinator_profile_path, profile, MANIFEST_ACTIVATION_FLAG, NULL, NULL);

    if(manifest == NULL)
    {
        g_printerr("Cannot open manifest file!\n");
        exit_status = 1;
    }
    else
    {
        if(check_manifest(manifest))
        {
            unsigned int i;
            GPtrArray *candidate_mappings_array = g_ptr_array_new();

            /* Look for all mappings that have the provided service name and (optionally) the container and target */
            for(i = 0; i < manifest->activation_array->len; i++)
            {
                ActivationMapping *mapping = (ActivationMapping*)g_ptr_array_index(manifest->activation_array, i);

                if(xmlStrcmp((const xmlChar*) service_name, mapping->name) == 0 &&
                  (container_filter == NULL || xmlStrcmp((const xmlChar*) container_filter, mapping->container) == 0) &&
                  (target_filter == NULL || xmlStrcmp((const xmlChar*) target_filter, mapping->target) == 0))
                     g_ptr_array_add(candidate_mappings_array, mapping);
            }

            if(show_mappings)
            {
                print_mappings(candidate_mappings_array);
                exit_status = 0;
            }
            else if(candidate_mappings_array->len == 0)
            {
                 g_printerr("No mapping found that matches the provided selection criteria!\n");
                 exit_status = 1;
            }
            else if(candidate_mappings_array->len == 1)
            {
                ActivationMapping *mapping = g_ptr_array_index(candidate_mappings_array, 0);
                exit_status = spawn_shell(manifest->targets_table, mapping, command);
            }
            else
            {
                if(command == NULL)
                {
                    g_printerr("Multiple mappings found! Please specify a --target and, optionally, a\n");
                    g_printerr("--container parameter! Alternatively, you can execute commands for all possible\n");
                    g_printerr("service mappings by providing a --command parameter.\n\n");

                    print_mappings(candidate_mappings_array);

                    exit_status = 1;
                }
                else
                {
                    for(i = 0; i < candidate_mappings_array->len; i++)
                    {
                        ActivationMapping *mapping = g_ptr_array_index(candidate_mappings_array, i);
                        exit_status = spawn_shell(manifest->targets_table, mapping, command);

                        if(exit_status != 0)
                            break;
                    }
                }
            }

            g_ptr_array_free(candidate_mappings_array, TRUE);
        }
        else
            exit_status = 1;

        delete_manifest(manifest);
    }

    return exit_status;
}
