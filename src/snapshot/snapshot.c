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

#include "snapshot.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <client-interface.h>
#include <manifest.h>
#include <snapshotmapping.h>
#include <targets.h>

static pid_t take_snapshot_on_target(SnapshotMapping *mapping, Target *target, gchar **arguments, unsigned int arguments_length)
{
    g_print("[target: %s]: Snapshotting state of service: %s\n", mapping->target, mapping->component);
    return exec_snapshot(target->client_interface, mapping->target, mapping->container, mapping->type, arguments, arguments_length, mapping->service);
}

static void complete_take_snapshot_on_target(SnapshotMapping *mapping, ProcReact_Status status, int result)
{
    if(status != PROCREACT_STATUS_OK || !result)
        g_printerr("[target: %s]: Cannot snapshot state of service: %s\n", mapping->target, mapping->component);
}

static int snapshot_services(GPtrArray *snapshots_array, GPtrArray *target_array)
{
    return map_snapshot_items(snapshots_array, target_array, take_snapshot_on_target, complete_take_snapshot_on_target);
}

typedef struct
{
    GPtrArray *snapshots_array;
    unsigned int flags;
}
RetrieveSnapshotsData;

static pid_t retrieve_snapshot_mapping(SnapshotMapping *mapping, Target *target, const unsigned int flags)
{
    g_print("[target: %s]: Retrieving snapshots of component: %s deployed to container: %s\n", mapping->target, mapping->component, mapping->container);
    return exec_copy_snapshots_from(target->client_interface, mapping->target, mapping->container, mapping->component, (flags & FLAG_ALL));
}

pid_t retrieve_snapshots_from_target(void *data, Target *target)
{
    pid_t pid = fork();
    
    if(pid == 0)
    {
        RetrieveSnapshotsData *retrieve_snapshots_data = (RetrieveSnapshotsData*)data;
        
        gchar *target_key = find_target_key(target);
        GPtrArray *snapshots_per_target_array = find_snapshot_mappings_per_target(retrieve_snapshots_data->snapshots_array, target_key);
        unsigned int i;
        int exit_status = 0;
        ProcReact_Status status;
        
        for(i = 0; i < snapshots_per_target_array->len; i++)
        {
            SnapshotMapping *mapping = g_ptr_array_index(snapshots_per_target_array, i);
            exit_status = procreact_wait_for_exit_status(retrieve_snapshot_mapping(mapping, target, retrieve_snapshots_data->flags), &status);
        
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

void complete_retrieve_snapshots_from_target(void *data, Target *target, ProcReact_Status status, int result)
{
    if(status != PROCREACT_STATUS_OK || !result)
    {
        gchar *target_key = find_target_key(target);
        g_printerr("[target: %s]: Cannot send snapshots!\n", target_key);
    }
}

static int retrieve_snapshots(GPtrArray *snapshots_array, GPtrArray *target_array, const unsigned int max_concurrent_transfers, const unsigned int flags)
{
    int success;
    RetrieveSnapshotsData data = { snapshots_array, flags };
    ProcReact_PidIterator iterator = create_target_iterator(target_array, retrieve_snapshots_from_target, complete_retrieve_snapshots_from_target, &data);
    
    g_print("[coordinator]: Retrieving snapshots...\n");
    
    procreact_fork_and_wait_in_parallel_limit(&iterator, max_concurrent_transfers);
    success = target_iterator_has_succeeded(&iterator);
    
    destroy_target_iterator(&iterator);
    
    return success;
}

static void cleanup(const unsigned int flags, const gchar *manifest_file, char *old_manifest_file, Manifest *manifest, GPtrArray *snapshots_array, GPtrArray *old_snapshots_array)
{
    g_free(old_manifest_file);
    
    if(!(flags & FLAG_NO_UPGRADE) && manifest_file != NULL)
    {
        delete_snapshots_array(old_snapshots_array);
        if(snapshots_array != NULL)
            g_ptr_array_free(snapshots_array, TRUE);
    }
    
    delete_manifest(manifest);
}

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
TakeRetrieveAndCleanSnapshotsData;

static pid_t take_retrieve_and_clean_snapshot_on_target(void *data, Target *target)
{
    pid_t pid = fork();
    
    if(pid == 0)
    {
        TakeRetrieveAndCleanSnapshotsData *retrieve_snapshots_data = (TakeRetrieveAndCleanSnapshotsData*)data;
        
        gchar *target_key = find_target_key(target);
        GPtrArray *snapshots_per_target_array = find_snapshot_mappings_per_target(retrieve_snapshots_data->snapshots_array, target_key);
        unsigned int i;
        int exit_status = 0;
        ProcReact_Status status;
        
        for(i = 0; i < snapshots_per_target_array->len; i++)
        {
            SnapshotMapping *mapping = g_ptr_array_index(snapshots_per_target_array, i);
            gchar **arguments = generate_activation_arguments(target, mapping->container); /* Generate an array of key=value pairs from container properties */
            unsigned int arguments_length = g_strv_length(arguments); /* Determine length of the activation arguments array */
            
            if(!procreact_wait_for_boolean(take_snapshot_on_target(mapping, target, arguments, arguments_length), &status) || (status != PROCREACT_STATUS_OK)
              || !procreact_wait_for_boolean(retrieve_snapshot_mapping(mapping, target, retrieve_snapshots_data->flags), &status) || (status != PROCREACT_STATUS_OK)
              || !procreact_wait_for_boolean(clean_snapshot_mapping(mapping, target, retrieve_snapshots_data->keep), &status) || (status != PROCREACT_STATUS_OK))
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

void complete_take_retrieve_and_clean_snapshots_on_target(void *data, Target *target, ProcReact_Status status, int result)
{
    if(status != PROCREACT_STATUS_OK || !result)
    {
        gchar *target_key = find_target_key(target);
        g_printerr("[target: %s]: Cannot take, send or clean snapshots!\n", target_key);
    }
}

static int snapshot_depth_first(GPtrArray *snapshots_array, GPtrArray *target_array, const unsigned int max_concurrent_transfers, const unsigned int flags, const int keep)
{
    int success;
    TakeRetrieveAndCleanSnapshotsData data = { snapshots_array, flags, keep };
    ProcReact_PidIterator iterator = create_target_iterator(target_array, take_retrieve_and_clean_snapshot_on_target, complete_take_retrieve_and_clean_snapshots_on_target, &data);
    
    g_print("[coordinator]: Snapshotting, retrieving and cleaning snapshots...\n");
    
    procreact_fork_and_wait_in_parallel_limit(&iterator, max_concurrent_transfers);
    success = target_iterator_has_succeeded(&iterator);
    
    destroy_target_iterator(&iterator);
    
    return success;
}

int snapshot(const gchar *manifest_file, const unsigned int max_concurrent_transfers, const unsigned int flags, const int keep, const gchar *old_manifest, const gchar *coordinator_profile_path, gchar *profile, const gchar *container_filter, const gchar *component_filter)
{
    /* Generate a distribution array from the manifest file */
    Manifest *manifest = open_provided_or_previous_manifest_file(manifest_file, coordinator_profile_path, profile, MANIFEST_SNAPSHOT_FLAG, container_filter, component_filter);
    
    if(manifest == NULL)
    {
        g_print("[coordinator]: Error while opening manifest file!\n");
        return 1;
    }
    else
    {
        GPtrArray *snapshots_array = NULL;
        GPtrArray *old_snapshots_array = NULL;
        gchar *old_manifest_file;
        
        if(old_manifest == NULL)
            old_manifest_file = determine_previous_manifest_file(coordinator_profile_path, profile);
        else
            old_manifest_file = g_strdup(old_manifest);
        
        if(!(flags & FLAG_NO_UPGRADE) && old_manifest_file == NULL)
            g_printerr("[coordinator]: No snapshots are taken since an upgrade is requested and no previous deployment state is known\n");
        else
        {
            if((flags & FLAG_NO_UPGRADE) || manifest_file == NULL)
            {
                g_printerr("[coordinator]: Snapshotting state of all components...\n");
                snapshots_array = manifest->snapshots_array;
            }
            else
            {
                old_snapshots_array = create_snapshots_array(old_manifest_file, container_filter, component_filter);
                g_printerr("[coordinator]: Snapshotting state of moved components using previous manifest: %s\n", old_manifest_file);
                snapshots_array = subtract_snapshot_mappings(old_snapshots_array, manifest->snapshots_array);
            }
        
            if(flags & FLAG_DEPTH_FIRST)
            {
                int exit_status;
                
                if(snapshot_depth_first(snapshots_array, manifest->target_array, max_concurrent_transfers, flags, keep))
                    exit_status = 0;
                else
                    exit_status = 1;
                
                cleanup(flags, manifest_file, old_manifest_file, manifest, snapshots_array, old_snapshots_array);
                return exit_status;
            }
            else
            {
                if((!(flags & FLAG_TRANSFER_ONLY) && !snapshot_services(snapshots_array, manifest->target_array)) /* First, take snapshots on the remote machines */
                  || (!retrieve_snapshots(snapshots_array, manifest->target_array, max_concurrent_transfers, flags))) /* Then transfer the snapshots to the coordinator machine */
                {
                    cleanup(flags, manifest_file, old_manifest_file, manifest, snapshots_array, old_snapshots_array);
                    return 1;
                }
            }
        }
        
        cleanup(flags, manifest_file, old_manifest_file, manifest, snapshots_array, old_snapshots_array);
        return 0;
    }
}
