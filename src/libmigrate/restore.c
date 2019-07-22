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
#include <client-interface.h>
#include <snapshotmappingarray.h>
#include <targets-iterator.h>
#include <nixxml-generate-env.h>

/* Send snapshots infrastructure */

typedef struct
{
    GPtrArray *snapshot_mapping_array;
    unsigned int flags;
}
SendSnapshotsData;

static pid_t send_snapshot_mapping(SnapshotMapping *mapping, Target *target, const unsigned int flags)
{
    g_print("[target: %s]: Sending snapshots of component: %s deployed to container: %s\n", mapping->target, mapping->component, mapping->container);
    return exec_copy_snapshots_to((gchar*)target->client_interface, (gchar*)mapping->target, (gchar*)mapping->container, (gchar*)mapping->component, (flags & FLAG_ALL));
}

pid_t send_snapshots_to_target(void *data, Target *target, gchar *client_interface, gchar *target_key)
{
    pid_t pid = fork();

    if(pid == 0)
    {
        SendSnapshotsData *send_snapshots_data = (SendSnapshotsData*)data;

        gchar *target_key = find_target_key(target, NULL);
        GPtrArray *snapshots_per_target_array = find_snapshot_mappings_per_target(send_snapshots_data->snapshot_mapping_array, target_key);
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

void complete_send_snapshots_to_target(void *data, Target *target, gchar *target_key, ProcReact_Status status, int result)
{
    if(status != PROCREACT_STATUS_OK || !result)
    {
        gchar *target_key = find_target_key(target, NULL);
        g_printerr("[target: %s]: Cannot retrieve snapshots!\n", target_key);
    }
}

static int send_snapshots(GPtrArray *snapshot_mapping_array, GHashTable *targets_table, const unsigned int max_concurrent_transfers, const unsigned int flags)
{
    int success;
    SendSnapshotsData data = { snapshot_mapping_array, flags };
    ProcReact_PidIterator iterator = create_target_pid_iterator(targets_table, NULL, NULL, send_snapshots_to_target, complete_send_snapshots_to_target, &data);
    procreact_fork_and_wait_in_parallel_limit(&iterator, max_concurrent_transfers);
    success = target_iterator_has_succeeded(iterator.data);

    destroy_target_pid_iterator(&iterator);

    return success;
}

/* Restore snapshot infrastructure */

static pid_t restore_snapshot_on_target(SnapshotMapping *mapping, ManifestService *service, Target *target, xmlChar **arguments, unsigned int arguments_length)
{
    g_print("[target: %s]: Restoring state of service: %s\n", mapping->target, mapping->component);
    return exec_restore((char*)target->client_interface, (char*)mapping->target, (char*)mapping->container, (char*)service->type, (char**)arguments, arguments_length, (char*)service->pkg);
}

static void complete_restore_snapshot_on_target(SnapshotMapping *mapping, Target *target, ProcReact_Status status, int result)
{
    if(status != PROCREACT_STATUS_OK || !result)
        g_printerr("[target: %s]: Cannot restore state of service: %s\n", mapping->target, mapping->component);
}

static int restore_services(GPtrArray *snapshot_mapping_array, GHashTable *services_table, GHashTable *targets_table)
{
    g_print("[coordinator]: Restoring state of services...\n");
    return map_snapshot_items(snapshot_mapping_array, services_table, targets_table, restore_snapshot_on_target, complete_restore_snapshot_on_target);
}

/* Clean snapshot mapping infrastructure */

static pid_t clean_snapshot_mapping(SnapshotMapping *mapping, Target *target, int keep)
{
    g_print("[target: %s]: Cleaning snapshots of component: %s deployed to container: %s\n", mapping->target, mapping->component, mapping->container);
    return exec_clean_snapshots((char*)target->client_interface, (char*)mapping->target, keep, (char*)mapping->container, (char*)mapping->component);
}

typedef struct
{
    GHashTable *services_table;
    GPtrArray *snapshot_mapping_array;
    unsigned int flags;
    int keep;
}
SendRestoreAndCleanSnapshotsData;

/* Restore depth-first infrastructure */

static pid_t send_restore_and_clean_snapshot_on_target(void *data, Target *target, gchar *client_interface, gchar *target_key)
{
    pid_t pid = fork();

    if(pid == 0)
    {
        SendRestoreAndCleanSnapshotsData *send_snapshots_data = (SendRestoreAndCleanSnapshotsData*)data;

        gchar *target_key = find_target_key(target, NULL);
        GPtrArray *snapshots_per_target_array = find_snapshot_mappings_per_target(send_snapshots_data->snapshot_mapping_array, target_key);
        unsigned int i;
        int exit_status = 0;
        ProcReact_Status status;

        for(i = 0; i < snapshots_per_target_array->len; i++)
        {
            SnapshotMapping *mapping = g_ptr_array_index(snapshots_per_target_array, i);
            xmlChar **arguments = generate_activation_arguments(target, (gchar*)mapping->container); /* Generate an array of key=value pairs from container properties */
            unsigned int arguments_length = g_strv_length((gchar**)arguments); /* Determine length of the activation arguments array */
            ManifestService *service = g_hash_table_lookup(send_snapshots_data->services_table, (gchar*)mapping->service);

            if(!procreact_wait_for_boolean(send_snapshot_mapping(mapping, target, send_snapshots_data->flags), &status) || (status != PROCREACT_STATUS_OK)
              || !procreact_wait_for_boolean(restore_snapshot_on_target(mapping, service, target, arguments, arguments_length), &status) || (status != PROCREACT_STATUS_OK)
              || !procreact_wait_for_boolean(clean_snapshot_mapping(mapping, target, send_snapshots_data->keep), &status) || (status != PROCREACT_STATUS_OK))
            {
                exit_status = 1;
                NixXML_delete_env_variable_array(arguments);
                break;
            }

            NixXML_delete_env_variable_array(arguments);
        }

        g_ptr_array_free(snapshots_per_target_array, TRUE);

        exit(exit_status);
    }

    return pid;
}

void complete_send_restore_and_clean_snapshots_on_target(void *data, Target *target, gchar *target_key, ProcReact_Status status, int result)
{
    if(status != PROCREACT_STATUS_OK || !result)
    {
        gchar *target_key = find_target_key(target, NULL);
        g_printerr("[target: %s]: Cannot send, restore or clean snapshots!\n", target_key);
    }
}

static int restore_depth_first(GPtrArray *snapshot_mapping_array, GHashTable *services_table, GHashTable *targets_table, const unsigned int max_concurrent_transfers, const unsigned int flags, const int keep)
{
    int success;
    SendRestoreAndCleanSnapshotsData data = { services_table, snapshot_mapping_array, flags, keep };
    ProcReact_PidIterator iterator = create_target_pid_iterator(targets_table, NULL, NULL, send_restore_and_clean_snapshot_on_target, complete_send_restore_and_clean_snapshots_on_target, &data);

    g_print("[coordinator]: Sending, restoring and cleaning snapshots...\n");

    procreact_fork_and_wait_in_parallel_limit(&iterator, max_concurrent_transfers);
    success = target_iterator_has_succeeded(iterator.data);

    destroy_target_pid_iterator(&iterator);

    return success;
}

/* The entire restore operation */

int restore(const Manifest *manifest, const Manifest *previous_manifest, const unsigned int max_concurrent_transfers, const unsigned int flags, const unsigned int keep)
{
    int exit_status;
    GPtrArray *snapshot_mapping_array;

    if(flags & FLAG_NO_UPGRADE || previous_manifest == NULL)
    {
        g_printerr("[coordinator]: Sending snapshots of all components...\n");
        snapshot_mapping_array = manifest->snapshot_mapping_array;
    }
    else
    {
        g_printerr("[coordinator]: Snapshotting state of moved components...\n");
        snapshot_mapping_array = subtract_snapshot_mappings(manifest->snapshot_mapping_array, previous_manifest->snapshot_mapping_array);
    }

    if(flags & FLAG_DEPTH_FIRST)
        exit_status = restore_depth_first(snapshot_mapping_array, manifest->services_table, manifest->targets_table, max_concurrent_transfers, flags, keep);
    else
    {
        exit_status = send_snapshots(snapshot_mapping_array, manifest->targets_table, max_concurrent_transfers, flags) /* First, send the snapshots to the remote machines */
          && ((flags & FLAG_TRANSFER_ONLY) || restore_services(snapshot_mapping_array, manifest->services_table, manifest->targets_table)); /* Then, restore them on the remote machines */
    }

    if(!(flags & FLAG_NO_UPGRADE) && previous_manifest != NULL)
        g_ptr_array_free(snapshot_mapping_array, TRUE);

    return exit_status;
}
