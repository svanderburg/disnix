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

#include "servicemapping-traverse.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <nixxml-generate-env.h>

GPtrArray *find_interdependent_service_mappings(GHashTable *services_table, const GPtrArray *service_mapping_array, const ServiceMapping *mapping)
{
    GPtrArray *return_array = g_ptr_array_new();
    unsigned int i;

    /* For each service mapping, check whether there is a inter-dependency on the requested mapping */
    for(i = 0; i < service_mapping_array->len; i++)
    {
        ServiceMapping *current_mapping = g_ptr_array_index(service_mapping_array, i);
        ManifestService *service = g_hash_table_lookup(services_table, current_mapping->service);
        InterDependencyMapping *found_dependency = find_interdependency_mapping(service->depends_on, (InterDependencyMapping*)mapping);

        if(found_dependency != NULL)
            g_ptr_array_add(return_array, current_mapping);
    }

    return return_array;
}

static ServiceStatus attempt_to_map_service_mapping(ServiceMapping *mapping, GHashTable *services_table, Target *target, GHashTable *pid_table, service_mapping_function map_service_mapping)
{
    if(request_available_target_core(target)) /* Check if machine has any cores available, if not wait and try again later */
    {
        xmlChar **arguments = generate_activation_arguments(target, (gchar*)mapping->container); /* Generate an array of key=value pairs from container properties */
        unsigned int arguments_size = g_strv_length((gchar**)arguments); /* Determine length of the activation arguments array */
        ManifestService *service = g_hash_table_lookup(services_table, (gchar*)mapping->service);
        pid_t pid = map_service_mapping(mapping, service, target, arguments, arguments_size); /* Execute the activation operation asynchronously */

        /* Cleanup */
        NixXML_delete_env_variable_array(arguments);

        if(pid == -1)
        {
            g_printerr("[target: %s]: Cannot fork process for service: %s!\n", mapping->target, mapping->service);
            return SERVICE_ERROR;
        }
        else
        {
            gint *pid_ptr = g_malloc(sizeof(gint));
            *pid_ptr = pid;

            mapping->status = SERVICE_MAPPING_IN_PROGRESS; /* Mark service mapping as in progress */
            g_hash_table_insert(pid_table, pid_ptr, mapping); /* Add mapping to the pids table so that we can retrieve its status later */
            return SERVICE_IN_PROGRESS;
        }
    }
    else
        return SERVICE_WAIT;
}

static void wait_for_service_mapping_to_complete(GHashTable *pid_table, GHashTable *services_table, GHashTable *targets_table, complete_service_mapping_function complete_service_mapping)
{
    int wstatus;

    /* Wait for an activation/deactivation process to finish */
    pid_t pid = wait(&wstatus);

    if(pid > 0)
    {
        /* Find the corresponding service mapping and remove it from the pids table */
        ProcReact_Status status;
        int result = procreact_retrieve_boolean(pid, wstatus, &status);
        ServiceMapping *mapping = g_hash_table_lookup(pid_table, &pid);
        ManifestService *service = g_hash_table_lookup(services_table, (gchar*)mapping->service);
        Target *target = g_hash_table_lookup(targets_table, (gchar*)mapping->target);
        g_hash_table_remove(pid_table, &pid);

        /* Complete the service mapping */
        complete_service_mapping(mapping, service, target, status, result);

        /* Signal the target to make the CPU core available again */
        target = g_hash_table_lookup(targets_table, (gchar*)mapping->target);
        signal_available_target_core(target);
    }
}

ServiceStatus traverse_inter_dependency_mappings(GPtrArray *unified_service_mapping_array, GHashTable *unified_services_table, const InterDependencyMapping *key, GHashTable *targets_table, GHashTable *pid_table, service_mapping_function map_service_mapping)
{
    /* Retrieve the mapping from the union array */
    ServiceMapping *actual_mapping = find_service_mapping(unified_service_mapping_array, key);

    ManifestService *service = g_hash_table_lookup(unified_services_table, actual_mapping->service);

    /* First, activate all inter-dependency mappings */
    if(service->depends_on != NULL)
    {
        unsigned int i;
        ServiceStatus status;

        for(i = 0; i < service->depends_on->len; i++)
        {
            InterDependencyMapping *dependency_mapping = g_ptr_array_index(service->depends_on, i);
            status = traverse_inter_dependency_mappings(unified_service_mapping_array, unified_services_table, dependency_mapping, targets_table, pid_table, map_service_mapping);

            if(status != SERVICE_DONE)
                return status; /* If any of the inter-dependencies has not been activated yet, relay its status */
        }
    }

    /* Finally, activate the mapping itself if it is not activated yet */
    switch(actual_mapping->status)
    {
        case SERVICE_MAPPING_DEACTIVATED:
            {
                Target *target = g_hash_table_lookup(targets_table, (gchar*)actual_mapping->target);

                if(target == NULL)
                {
                    g_print("[target: %s]: Cannot map service with key: %s deploying service: %s since the machine is not present!\n", actual_mapping->service, actual_mapping->target, service->pkg);
                    return SERVICE_ERROR;
                }
                else
                    return attempt_to_map_service_mapping(actual_mapping, unified_services_table, target, pid_table, map_service_mapping);
            }
        case SERVICE_MAPPING_ACTIVATED:
            return SERVICE_DONE;
        case SERVICE_MAPPING_IN_PROGRESS:
            return SERVICE_IN_PROGRESS;
        default:
            return SERVICE_ERROR; /* Should never happen */
    }
}

ServiceStatus traverse_interdependent_mappings(GPtrArray *unified_service_mapping_array, GHashTable *unified_services_table, const InterDependencyMapping *key, GHashTable *targets_table, GHashTable *pid_table, service_mapping_function map_service_mapping)
{
    /* Retrieve the mapping from the union array */
    ServiceMapping *actual_mapping = find_service_mapping(unified_service_mapping_array, key);

    /* Find all interdependent mapping on this mapping */
    GPtrArray *interdependent_mappings = find_interdependent_service_mappings(unified_services_table, unified_service_mapping_array, actual_mapping);

    /* First deactivate all mappings which have an inter-dependency on this mapping */
    unsigned int i;

    for(i = 0; i < interdependent_mappings->len; i++)
    {
        ServiceMapping *dependency_mapping = g_ptr_array_index(interdependent_mappings, i);
        ServiceStatus status = traverse_interdependent_mappings(unified_service_mapping_array, unified_services_table, (InterDependencyMapping*)dependency_mapping, targets_table, pid_table, map_service_mapping);

        if(status != SERVICE_DONE)
        {
            g_ptr_array_free(interdependent_mappings, TRUE);
            return status; /* If any inter-dependency is not deactivated, relay its status */
        }
    }

    g_ptr_array_free(interdependent_mappings, TRUE);

    /* Finally deactivate the mapping itself if it has not been deactivated yet */
    switch(actual_mapping->status)
    {
        case SERVICE_MAPPING_ACTIVATED:
            {
                Target *target = g_hash_table_lookup(targets_table, (gchar*)actual_mapping->target);
                ManifestService *service = g_hash_table_lookup(unified_services_table, (gchar*)actual_mapping->service);

                if(target == NULL)
                {
                    g_print("[target: %s]: Skip service with key: %s deploying service: %s since machine is no longer present!\n", actual_mapping->target, actual_mapping->service, service->pkg);
                    actual_mapping->status = SERVICE_MAPPING_DEACTIVATED;
                    return SERVICE_DONE;
                }
                else
                    return attempt_to_map_service_mapping(actual_mapping, unified_services_table, target, pid_table, map_service_mapping);
            }
        case SERVICE_MAPPING_DEACTIVATED:
            return SERVICE_DONE;
        case SERVICE_MAPPING_IN_PROGRESS:
            return SERVICE_IN_PROGRESS;
        default:
            return SERVICE_ERROR; /* Should never happen */
    }
}

int traverse_service_mappings(GPtrArray *mappings, GPtrArray *unified_service_mapping_array, GHashTable *unified_services_table, GHashTable *targets_table, iterate_strategy_function iterate_strategy, service_mapping_function map_service_mapping, complete_service_mapping_function complete_service_mapping)
{
    GHashTable *pid_table = g_hash_table_new_full(g_int_hash, g_int_equal, g_free, NULL);
    unsigned int num_done = 0;
    int success = TRUE;

    do
    {
        unsigned int i;
        num_done = 0;

        for(i = 0; i < mappings->len; i++)
        {
            ServiceMapping *mapping = g_ptr_array_index(mappings, i);
            ServiceStatus status = iterate_strategy(unified_service_mapping_array, unified_services_table, (InterDependencyMapping*)mapping, targets_table, pid_table, map_service_mapping);

            if(status == SERVICE_ERROR)
            {
                success = FALSE;
                num_done++;
            }
            else if(status == SERVICE_DONE)
                num_done++;

            wait_for_service_mapping_to_complete(pid_table, unified_services_table, targets_table, complete_service_mapping);
        }
    }
    while(num_done < mappings->len);

    g_hash_table_destroy(pid_table);
    return success;
}
