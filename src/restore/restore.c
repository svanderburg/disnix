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

#include "restore.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <client-interface.h>
#include <manifest.h>
#include <snapshotmapping.h>
#include <targets.h>

typedef struct
{
    GPtrArray *snapshots_array;
    int all;
}
SendSnapshotsData;

pid_t send_snapshots_to_target(void *data, Target *target)
{
    pid_t pid = fork();
    
    if(pid == 0)
    {
        SendSnapshotsData *send_snapshots_data = (SendSnapshotsData*)data;
        
        gchar *target_key = find_target_key(target);
        GPtrArray *snapshots_per_target_array = find_snapshot_mappings_per_target(send_snapshots_data->snapshots_array, target_key);
        unsigned int i;
        int exit_status = 0;
        ProcReact_Status status;
        
        for(i = 0; i < snapshots_per_target_array->len; i++)
        {
            SnapshotMapping *mapping = g_ptr_array_index(snapshots_per_target_array, i);
        
            g_print("[target: %s]: Sending snapshots of component: %s deployed to container: %s\n", mapping->target, mapping->component, mapping->container);
            exit_status = procreact_wait_for_exit_status(exec_copy_snapshots_to(target->client_interface, mapping->target, mapping->container, mapping->component, send_snapshots_data->all), &status);
        
            if(status != PROCREACT_STATUS_OK)
            {
                exit_status = 1;
                break;
            }
            else if(exit_status != 0)
                break;
        }
    
        g_ptr_array_free(snapshots_per_target_array, TRUE);
        
        exit(exit_status);
    }
    
    return pid;
}

void complete_send_snapshots_to_target(void *data, Target *target, ProcReact_Status status, int result)
{
    if(status != PROCREACT_STATUS_OK || !result)
    {
        gchar *target_key = find_target_key(target);
        g_printerr("[target: %s]: Cannot retrieve snapshots!\n", target_key);
    }
}

static int send_snapshots(GPtrArray *snapshots_array, GPtrArray *target_array, const unsigned int max_concurrent_transfers, const int all)
{
    int success;
    SendSnapshotsData data = { snapshots_array, all };
    ProcReact_PidIterator iterator = create_target_iterator(target_array, send_snapshots_to_target, complete_send_snapshots_to_target, &data);
    procreact_fork_and_wait_in_parallel_limit(&iterator, max_concurrent_transfers);
    success = target_iterator_has_succeeded(&iterator);
    
    destroy_target_iterator(&iterator);
    
    return success;
}

static pid_t restore_snapshot_on_target(SnapshotMapping *mapping, Target *target, gchar **arguments, unsigned int arguments_length)
{
    g_print("[target: %s]: Restoring state of service: %s\n", mapping->target, mapping->component);
    return exec_restore(target->client_interface, mapping->target, mapping->container, mapping->type, arguments, arguments_length, mapping->service);
}

static void complete_restore_snapshot_on_target(SnapshotMapping *mapping, ProcReact_Status status, int result)
{
    if(status != PROCREACT_STATUS_OK || !result)
        g_printerr("[target: %s]: Cannot restore state of service: %s\n", mapping->target, mapping->component);
}

static int restore_services(GPtrArray *snapshots_array, GPtrArray *target_array)
{
    g_print("[coordinator]: Restoring state of services...\n");
    return map_snapshot_items(snapshots_array, target_array, restore_snapshot_on_target, complete_restore_snapshot_on_target);
}

int restore(const gchar *manifest_file, const unsigned int max_concurrent_transfers, const int transfer_only, const int all, const gchar *old_manifest, const gchar *coordinator_profile_path, gchar *profile, const gboolean no_upgrade, const gchar *container_filter, const gchar *component_filter)
{
    /* Generate a distribution array from the manifest file */
    Manifest *manifest = open_provided_or_previous_manifest_file(manifest_file, coordinator_profile_path, profile, container_filter, component_filter);
    
    if(manifest == NULL)
    {
        g_print("[coordinator]: Error while opening manifest file!\n");
        return 1;
    }
    else
    {
        int exit_status;
        GPtrArray *snapshots_array;
        gchar *old_manifest_file;
        
        if(old_manifest == NULL)
            old_manifest_file = determine_previous_manifest_file(coordinator_profile_path, profile);
        else
            old_manifest_file = g_strdup(old_manifest);
        
        if(no_upgrade || old_manifest_file == NULL)
        {
            g_printerr("[coordinator]: Sending snapshots of all components...\n");
            snapshots_array = manifest->snapshots_array;
        }
        else
        {
            GPtrArray *old_snapshots_array = create_snapshots_array(old_manifest_file, container_filter, component_filter);
            g_printerr("[coordinator]: Snapshotting state of moved components...\n");
            snapshots_array = subtract_snapshot_mappings(manifest->snapshots_array, old_snapshots_array);
            delete_snapshots_array(old_snapshots_array);
        }
        
        if(send_snapshots(snapshots_array, manifest->target_array, max_concurrent_transfers, all) /* First, send the snapshots to the remote machines */
          && (transfer_only || restore_services(snapshots_array, manifest->target_array))) /* Then, restore them on the remote machines */
            exit_status = 0;
        else
            exit_status = 1;
        
        /* Cleanup */
        g_free(old_manifest_file);
        
        if(!no_upgrade && old_manifest_file != NULL)
            g_ptr_array_free(snapshots_array, TRUE);
    
        delete_manifest(manifest);

        /* Return the exit status */
        return exit_status;
    }
}
