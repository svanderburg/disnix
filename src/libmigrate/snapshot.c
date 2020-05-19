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

#include "snapshot.h"
#include <sys/types.h>
#include <client-interface.h>
#include <snapshotmapping-traverse.h>
#include <manifestservicestable.h>
#include <targets-iterator.h>
#include <mappingparameters.h>
#include <copy-snapshots.h>

/* Snapshot services infrastructure */

static pid_t take_snapshot_on_target(SnapshotMapping *mapping, ManifestService *service, Target *target, xmlChar *type, xmlChar **arguments, unsigned int arguments_length)
{
    gchar *target_key = find_target_key(target);
    g_print("[target: %s]: Snapshotting state of service: %s\n", mapping->target, mapping->component);
    return exec_snapshot((char*)target->client_interface, target_key, (char*)mapping->container, (char*)type, (char**)arguments, arguments_length, (char*)service->pkg);
}

static void complete_take_snapshot_on_target(SnapshotMapping *mapping, ManifestService *service, Target *target, ProcReact_Status status, int result)
{
    if(status != PROCREACT_STATUS_OK || !result)
        g_printerr("[target: %s]: Cannot snapshot state of service: %s\n", mapping->target, mapping->component);
}

static int snapshot_services(GPtrArray *snapshots_array, GHashTable *services_table, GHashTable *targets_table)
{
    return map_snapshot_items(snapshots_array, services_table, targets_table, take_snapshot_on_target, complete_take_snapshot_on_target);
}

/* Retrieve snapshots infrastructure */

typedef struct
{
    GPtrArray *snapshots_array;
    unsigned int flags;
}
RetrieveSnapshotsData;

static pid_t retrieve_snapshot_mapping(SnapshotMapping *mapping, Target *target, const unsigned int flags)
{
    gchar *target_key = find_target_key(target);
    g_print("[target: %s]: Retrieving snapshots of component: %s deployed to container: %s\n", mapping->target, mapping->component, mapping->container);
    //return exec_copy_snapshots_from((char*)target->client_interface, target_key, (char*)mapping->container, (char*)mapping->component, (flags & FLAG_ALL));
    return copy_snapshots_from((char*)target->client_interface, target_key, (char*)mapping->container, (char*)mapping->component, flags & FLAG_ALL);
}

pid_t retrieve_snapshots_from_target(void *data, gchar *target_name, Target *target)
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

void complete_retrieve_snapshots_from_target(void *data, gchar *target_name, Target *target, ProcReact_Status status, int result)
{
    if(status != PROCREACT_STATUS_OK || !result)
        g_printerr("[target: %s]: Cannot send snapshots!\n", target_name);
}

static int retrieve_snapshots(GPtrArray *snapshots_array, GHashTable *targets_table, const unsigned int max_concurrent_transfers, const unsigned int flags)
{
    int success;
    RetrieveSnapshotsData data = { snapshots_array, flags };
    ProcReact_PidIterator iterator = create_target_pid_iterator(targets_table, retrieve_snapshots_from_target, complete_retrieve_snapshots_from_target, &data);

    g_print("[coordinator]: Retrieving snapshots...\n");

    procreact_fork_and_wait_in_parallel_limit(&iterator, max_concurrent_transfers);
    success = target_iterator_has_succeeded(iterator.data);

    destroy_target_pid_iterator(&iterator);

    return success;
}

/* Clean snapshot mapping infrastructure */

static pid_t clean_snapshot_mapping(SnapshotMapping *mapping, Target *target, int keep)
{
    gchar *target_key = find_target_key(target);
    g_print("[target: %s]: Cleaning snapshots of component: %s deployed to container: %s\n", mapping->target, mapping->component, mapping->container);
    return exec_clean_snapshots((char*)target->client_interface, target_key, keep, (char*)mapping->container, (char*)mapping->component);
}

typedef struct
{
    GHashTable *services_table;
    GPtrArray *snapshot_mapping_array;
    unsigned int flags;
    int keep;
}
TakeRetrieveAndCleanSnapshotsData;

/* Snapshot depth-first infrastructure */

static pid_t take_retrieve_and_clean_snapshot_on_target(void *data, gchar *target_name, Target *target)
{
    pid_t pid = fork();

    if(pid == 0)
    {
        TakeRetrieveAndCleanSnapshotsData *retrieve_snapshots_data = (TakeRetrieveAndCleanSnapshotsData*)data;

        gchar *target_key = find_target_key(target);
        GPtrArray *snapshots_per_target_array = find_snapshot_mappings_per_target(retrieve_snapshots_data->snapshot_mapping_array, target_key);
        unsigned int i;
        int exit_status = 0;
        ProcReact_Status status;

        for(i = 0; i < snapshots_per_target_array->len; i++)
        {
            SnapshotMapping *mapping = g_ptr_array_index(snapshots_per_target_array, i);

            MappingParameters params = create_mapping_parameters(mapping->service, mapping->container, mapping->target, retrieve_snapshots_data->services_table, target);

            if(!procreact_wait_for_boolean(take_snapshot_on_target(mapping, params.service, target, params.type, params.arguments, params.arguments_size), &status) || (status != PROCREACT_STATUS_OK)
              || !procreact_wait_for_boolean(retrieve_snapshot_mapping(mapping, target, retrieve_snapshots_data->flags), &status) || (status != PROCREACT_STATUS_OK)
              || !procreact_wait_for_boolean(clean_snapshot_mapping(mapping, target, retrieve_snapshots_data->keep), &status) || (status != PROCREACT_STATUS_OK))
            {
                exit_status = 1;
                destroy_mapping_parameters(&params);
                break;
            }

            destroy_mapping_parameters(&params);
        }

        g_ptr_array_free(snapshots_per_target_array, TRUE);

        exit(exit_status);
    }

    return pid;
}

void complete_take_retrieve_and_clean_snapshots_on_target(void *data, gchar *target_name, Target *target, ProcReact_Status status, int result)
{
    if(status != PROCREACT_STATUS_OK || !result)
        g_printerr("[target: %s]: Cannot take, send or clean snapshots!\n", target_name);
}

static int snapshot_depth_first(GPtrArray *snapshot_mapping_array, GHashTable *services_table, GHashTable *targets_table, const unsigned int max_concurrent_transfers, const unsigned int flags, const int keep)
{
    int success;
    TakeRetrieveAndCleanSnapshotsData data = { services_table, snapshot_mapping_array, flags, keep };
    ProcReact_PidIterator iterator = create_target_pid_iterator(targets_table, take_retrieve_and_clean_snapshot_on_target, complete_take_retrieve_and_clean_snapshots_on_target, &data);

    g_print("[coordinator]: Snapshotting, retrieving and cleaning snapshots...\n");

    procreact_fork_and_wait_in_parallel_limit(&iterator, max_concurrent_transfers);
    success = target_iterator_has_succeeded(iterator.data);

    destroy_target_pid_iterator(&iterator);

    return success;
}

/* The entire snapshot operation */

int snapshot(const Manifest *manifest, const Manifest *previous_manifest, const unsigned int max_concurrent_transfers, const unsigned int flags, const int keep)
{
    if(!(flags & FLAG_NO_UPGRADE) && previous_manifest == NULL)
    {
        g_printerr("[coordinator]: No snapshots are taken since an upgrade is requested and no previous deployment state is known\n");
        return TRUE;
    }
    else
    {
        GPtrArray *snapshot_mapping_array = NULL;
        GPtrArray *previous_snapshot_mapping_array;
        GHashTable *previous_services_table;
        int exit_status;

        if(previous_manifest == NULL)
        {
            previous_snapshot_mapping_array = NULL;
            previous_services_table = manifest->services_table;
        }
        else
        {
            previous_snapshot_mapping_array = previous_manifest->snapshot_mapping_array;
            previous_services_table = previous_manifest->services_table;
        }

        if(flags & FLAG_NO_UPGRADE)
        {
            g_printerr("[coordinator]: Snapshotting state of all components...\n");
            snapshot_mapping_array = manifest->snapshot_mapping_array;
        }
        else
        {
            g_printerr("[coordinator]: Snapshotting state of moved components...\n");
            snapshot_mapping_array = subtract_snapshot_mappings(previous_snapshot_mapping_array, manifest->snapshot_mapping_array);
        }

        if(flags & FLAG_DEPTH_FIRST)
            exit_status = snapshot_depth_first(snapshot_mapping_array, previous_services_table, manifest->targets_table, max_concurrent_transfers, flags, keep);
        else
        {
            exit_status = ((flags & FLAG_TRANSFER_ONLY) || snapshot_services(snapshot_mapping_array, previous_services_table, manifest->targets_table))
              && retrieve_snapshots(snapshot_mapping_array, manifest->targets_table, max_concurrent_transfers, flags);
        }

        if(!(flags & FLAG_NO_UPGRADE))
            g_ptr_array_free(snapshot_mapping_array, TRUE);

        return exit_status;
    }
}
