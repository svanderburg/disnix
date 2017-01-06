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

#include "delete-state.h"
#include <client-interface.h>
#include <manifest.h>
#include <snapshotmapping.h>
#include <targets.h>

static pid_t delete_state_on_target(SnapshotMapping *mapping, Target *target, gchar **arguments, unsigned int arguments_length)
{
    g_print("[target: %s]: Deleting obsolete state of service: %s\n", mapping->target, mapping->component);
    return exec_delete_state(target->client_interface, mapping->target, mapping->container, mapping->type, arguments, arguments_length, mapping->service);
}

static void complete_delete_state_on_target(SnapshotMapping *mapping, ProcReact_Status status, int result)
{
    if(status != PROCREACT_STATUS_OK || !result)
        g_printerr("[target: %s]: Cannot delete state of service: %s\n", mapping->target, mapping->component);
}

static int delete_obsolete_state(GPtrArray *snapshots_array, GPtrArray *target_array)
{
    return map_snapshot_items(snapshots_array, target_array, delete_state_on_target, complete_delete_state_on_target);
}

int delete_state(const gchar *manifest_file, const gchar *coordinator_profile_path, gchar *profile, const gchar *container, const gchar *component)
{
    Manifest *manifest = open_provided_or_previous_manifest_file(manifest_file, coordinator_profile_path, profile, MANIFEST_SNAPSHOT_FLAG, container, component);
    
    if(manifest == NULL)
    {
        g_printerr("[coordinator]: Error while opening manifest file!\n");
        g_printerr("Please provde a valid manifest file as a command-line parameter.\n");
        return 1;
    }
    else
    {
        int exit_status;
        
        /* Delete the obsolete state */
        g_printerr("[coordinator]: Deleting obsolete state of services...\n");
        exit_status = !delete_obsolete_state(manifest->snapshots_array, manifest->target_array);
        delete_manifest(manifest);
        return exit_status;
    }
}
