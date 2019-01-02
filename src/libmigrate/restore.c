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

#include "restore.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <client-interface.h>
#include <snapshotmapping.h>
#include <targets.h>

/* Send snapshots infrastructure */

typedef struct
{
    GPtrArray *snapshots_array;
    unsigned int flags;
}
SendSnapshotsData;

static pid_t send_snapshot_mapping(SnapshotMapping *mapping, Target *target, const unsigned int flags)
{
    g_print("[target: %s]: Sending snapshots of component: %s deployed to container: %s\n", mapping->target, mapping->component, mapping->container);
    return exec_copy_snapshots_to(target->client_interface, mapping->target, mapping->container, mapping->component, (flags & FLAG_ALL));
}

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
            exit_status = procreact_wait_for_exit_status(send_snapshot_mapping(mapping, target, send_snapshots_data->flags), &status);
        
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

static int send_snapshots(GPtrArray *snapshots_array, GPtrArray *target_array, const unsigned int max_concurrent_transfers, const unsigned int flags)
{
    int success;
    SendSnapshotsData data = { snapshots_array, flags };
    ProcReact_PidIterator iterator = create_target_iterator(target_array, send_snapshots_to_target, complete_send_snapshots_to_target, &data);
    procreact_fork_and_wait_in_parallel_limit(&iterator, max_concurrent_transfers);
    success = target_iterator_has_succeeded(&iterator);
    
    destroy_target_iterator(&iterator);
    
    return success;
}

/* Restore snapshot infrastructure */

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

/* Clean snapshot mapping infrastructure */

static pid_t clean_snapshot_mapping(SnapshotMapping *mapping, Target *target, int keep)
{
    g_print("[target: %s]: Cleaning snapshots of component: %s deployed to container: %s\n", mapping->target, mapping->component, mapping->container);
    return exec_clean_snapshots(target->client_interface, mapping->target, keep, mapping->container, mapping->component);
}

typedef struct
{
    GPtrArray *snapshots_array;
    unsigned int flags;
    int keep;
}
SendRestoreAndCleanSnapshotsData;

/* Restore depth-first infrastructure */

static pid_t send_restore_and_clean_snapshot_on_target(void *data, Target *target)
{
    pid_t pid = fork();
    
    if(pid == 0)
    {
        SendRestoreAndCleanSnapshotsData *send_snapshots_data = (SendRestoreAndCleanSnapshotsData*)data;
        
        gchar *target_key = find_target_key(target);
        GPtrArray *snapshots_per_target_array = find_snapshot_mappings_per_target(send_snapshots_data->snapshots_array, target_key);
        unsigned int i;
        int exit_status = 0;
        ProcReact_Status status;
        
        for(i = 0; i < snapshots_per_target_array->len; i++)
        {
            SnapshotMapping *mapping = g_ptr_array_index(snapshots_per_target_array, i);
            gchar **arguments = generate_activation_arguments(target, mapping->container); /* Generate an array of key=value pairs from container properties */
            unsigned int arguments_length = g_strv_length(arguments); /* Determine length of the activation arguments array */
            
            if(!procreact_wait_for_boolean(send_snapshot_mapping(mapping, target, send_snapshots_data->flags), &status) || (status != PROCREACT_STATUS_OK)
              || !procreact_wait_for_boolean(restore_snapshot_on_target(mapping, target, arguments, arguments_length), &status) || (status != PROCREACT_STATUS_OK)
              || !procreact_wait_for_boolean(clean_snapshot_mapping(mapping, target, send_snapshots_data->keep), &status) || (status != PROCREACT_STATUS_OK))
            {
                exit_status = 1;
                g_strfreev(arguments);
                break;
            }
            
            g_strfreev(arguments);
        }
    
        g_ptr_array_free(snapshots_per_target_array, TRUE);
        
        exit(exit_status);
    }
    
    return pid;
}

void complete_send_restore_and_clean_snapshots_on_target(void *data, Target *target, ProcReact_Status status, int result)
{
    if(status != PROCREACT_STATUS_OK || !result)
    {
        gchar *target_key = find_target_key(target);
        g_printerr("[target: %s]: Cannot send, restore or clean snapshots!\n", target_key);
    }
}

static int restore_depth_first(GPtrArray *snapshots_array, GPtrArray *target_array, const unsigned int max_concurrent_transfers, const unsigned int flags, const int keep)
{
    int success;
    SendRestoreAndCleanSnapshotsData data = { snapshots_array, flags, keep };
    ProcReact_PidIterator iterator = create_target_iterator(target_array, send_restore_and_clean_snapshot_on_target, complete_send_restore_and_clean_snapshots_on_target, &data);
    
    g_print("[coordinator]: Sending, restoring and cleaning snapshots...\n");
    
    procreact_fork_and_wait_in_parallel_limit(&iterator, max_concurrent_transfers);
    success = target_iterator_has_succeeded(&iterator);
    
    destroy_target_iterator(&iterator);
    
    return success;
}

/* The entire restore operation */

int restore(const Manifest *manifest, GPtrArray *old_snapshots_array, const unsigned int max_concurrent_transfers, const unsigned int flags, const unsigned int keep)
{
    int exit_status;
    GPtrArray *snapshots_array;

    if(flags & FLAG_NO_UPGRADE || old_snapshots_array == NULL)
    {
        g_printerr("[coordinator]: Sending snapshots of all components...\n");
        snapshots_array = manifest->snapshots_array;
    }
    else
    {
        g_printerr("[coordinator]: Snapshotting state of moved components...\n");
        snapshots_array = subtract_snapshot_mappings(manifest->snapshots_array, old_snapshots_array);
    }

    if(flags & FLAG_DEPTH_FIRST)
        exit_status = restore_depth_first(snapshots_array, manifest->target_array, max_concurrent_transfers, flags, keep);
    else
    {
        exit_status = send_snapshots(snapshots_array, manifest->target_array, max_concurrent_transfers, flags) /* First, send the snapshots to the remote machines */
          && ((flags & FLAG_TRANSFER_ONLY) || restore_services(snapshots_array, manifest->target_array)); /* Then, restore them on the remote machines */
    }

    if(!(flags & FLAG_NO_UPGRADE) && old_snapshots_array != NULL)
        g_ptr_array_free(snapshots_array, TRUE);

    return exit_status;
}
