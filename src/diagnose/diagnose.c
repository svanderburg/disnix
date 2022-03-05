/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2022  Sander van der Burg
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
#include <servicemappingarray.h>
#include <manifestservice.h>
#include <targetstable.h>
#include <remote-state-management.h>
#include <procreact_pid.h>
#include <manifestservicestable.h>
#include <mappingparameters.h>

static int spawn_shell(GHashTable *services_table, GHashTable *targets_table, const ServiceMapping *mapping, const ManifestService *service, char *command)
{
    Target *target = g_hash_table_lookup(targets_table, (gchar*)mapping->target);
    gchar *target_key = find_target_key(target);

    MappingParameters params = create_mapping_parameters(mapping->service, mapping->container, mapping->target, mapping->container_provided_by_service, services_table, target);

    ProcReact_Status status;
    int exit_status;

    g_printerr("[%s]: Connecting to service: %s deployed to container: %s\n", mapping->target, service->pkg, mapping->container);

    exit_status = procreact_wait_for_exit_status(statemgmt_remote_shell((char*)target->client_interface, target_key, (char*)mapping->container, (char*)params.type, (char**)params.arguments, params.arguments_size, (char*)params.service->pkg, command), &status);

    destroy_mapping_parameters(&params);

    return exit_status;
}

static void print_mappings(const GPtrArray *mappings_array)
{
    unsigned int i;

    g_printerr("This service has been mapped to:\n\n");

    for(i = 0; i < mappings_array->len; i++)
    {
        ServiceMapping *mapping = (ServiceMapping*)g_ptr_array_index(mappings_array, i);
        g_printerr("container: %s, target: %s\n", mapping->container, mapping->target);
    }
}

int diagnose(const char *service_name, const int show_mappings, const char *manifest_file, const gchar *coordinator_profile_path, gchar *profile, const char *container_filter, const char *target_filter, char *command)
{
    int exit_status;
    Manifest *manifest = open_provided_or_previous_manifest_file(manifest_file, coordinator_profile_path, profile, MANIFEST_SERVICE_MAPPINGS_FLAG | MANIFEST_INFRASTRUCTURE_FLAG, NULL, NULL);

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
            for(i = 0; i < manifest->service_mapping_array->len; i++)
            {
                ServiceMapping *mapping = (ServiceMapping*)g_ptr_array_index(manifest->service_mapping_array, i);
                ManifestService *service = g_hash_table_lookup(manifest->services_table, mapping->service);

                if(xmlStrcmp((const xmlChar*) service_name, service->name) == 0 &&
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
                ServiceMapping *mapping = g_ptr_array_index(candidate_mappings_array, 0);
                ManifestService *service = g_hash_table_lookup(manifest->services_table, mapping->service);
                exit_status = spawn_shell(manifest->services_table, manifest->targets_table, mapping, service, command);
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
                        ServiceMapping *mapping = g_ptr_array_index(candidate_mappings_array, i);
                        ManifestService *service = g_hash_table_lookup(manifest->services_table, mapping->service);
                        exit_status = spawn_shell(manifest->services_table, manifest->targets_table, mapping, service, command);

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
