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

#include "snapshotmapping-traverse.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <nixxml-generate-env.h>
#include "interdependencymapping.h"
#include "manifestservicestable.h"
#include "mappingparameters.h"

static int wait_to_complete_snapshot_item(GHashTable *pid_table, GHashTable *services_table, GHashTable *targets_table, complete_snapshot_item_mapping_function complete_snapshot_item_mapping)
{
    if(g_hash_table_size(pid_table) > 0)
    {
        int wstatus;
        pid_t pid = wait(&wstatus);

        if(pid == -1)
            return FALSE;
        else
        {
            ManifestService *service;
            Target *target;
            ProcReact_Status status;
            int result;

            /* Find the corresponding snapshot mapping and remove it from the pids table */
            SnapshotMapping *mapping = g_hash_table_lookup(pid_table, &pid);
            g_hash_table_remove(pid_table, &pid);

            /* Mark mapping as transferred to prevent it from snapshotting again */
            mapping->transferred = TRUE;

            /* Signal the target to make the CPU core available again */
            target = g_hash_table_lookup(targets_table, (gchar*)mapping->target);
            signal_available_target_core(target);

            /* Return the status */
            result = procreact_retrieve_boolean(pid, wstatus, &status);
            service = g_hash_table_lookup(services_table, mapping->service);
            complete_snapshot_item_mapping(mapping, service, target, status, result);
            return(status == PROCREACT_STATUS_OK && result);
        }
    }
    else
        return TRUE;
}

int map_snapshot_items(const GPtrArray *snapshot_mapping_array, GHashTable *services_table, GHashTable *targets_table, map_snapshot_item_function map_snapshot_item, complete_snapshot_item_mapping_function complete_snapshot_item_mapping)
{
    unsigned int num_processed = 0;
    int status = TRUE;
    GHashTable *pid_table = g_hash_table_new_full(g_int_hash, g_int_equal, g_free, NULL);

    while(num_processed < snapshot_mapping_array->len)
    {
        unsigned int i;

        for(i = 0; i < snapshot_mapping_array->len; i++)
        {
            SnapshotMapping *mapping = g_ptr_array_index(snapshot_mapping_array, i);
            Target *target = g_hash_table_lookup(targets_table, (gchar*)mapping->target);

            if(target == NULL)
                g_print("[target: %s]: Skip state of component: %s deployed to container: %s since machine is no longer present!\n", mapping->target, mapping->component, mapping->container);
            else if(!mapping->transferred && request_available_target_core(target)) /* Check if machine has any cores available, if not wait and try again later */
            {
                MappingParameters params = create_mapping_parameters(mapping->service, mapping->container, mapping->target, services_table, target);
                pid_t pid = map_snapshot_item(mapping, params.service, target, params.type, params.arguments, params.arguments_size);

                /* Add pid and mapping to the hash table */
                gint *pid_ptr = g_malloc(sizeof(gint));
                *pid_ptr = pid;
                g_hash_table_insert(pid_table, pid_ptr, mapping);

                /* Cleanup */
                destroy_mapping_parameters(&params);
            }
        }

        if(!wait_to_complete_snapshot_item(pid_table, services_table, targets_table, complete_snapshot_item_mapping))
            status = FALSE;

        num_processed++;
    }

    g_hash_table_destroy(pid_table);
    return status;
}
