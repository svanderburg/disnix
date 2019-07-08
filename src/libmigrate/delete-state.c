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

#include "delete-state.h"
#include <client-interface.h>
#include <snapshotmappingarray.h>
#include <targetstable.h>

static pid_t delete_state_on_target(SnapshotMapping *mapping, ManifestService *service, Target *target, gchar **arguments, unsigned int arguments_length)
{
    g_print("[target: %s]: Deleting obsolete state of service: %s\n", mapping->target, mapping->component);
    return exec_delete_state((char*)target->client_interface, (char*)mapping->target, (char*)mapping->container, (char*)service->type, arguments, arguments_length, (char*)service->pkg);
}

static void complete_delete_state_on_target(SnapshotMapping *mapping, Target *target, ProcReact_Status status, int result)
{
    if(status != PROCREACT_STATUS_OK || !result)
        g_printerr("[target: %s]: Cannot delete state of service: %s\n", mapping->target, mapping->component);
}

int delete_obsolete_state(GPtrArray *snapshot_mapping_array, GHashTable *services_table, GHashTable *targets_table)
{
    reset_snapshot_items_transferred_status(snapshot_mapping_array);
    return map_snapshot_items(snapshot_mapping_array, services_table, targets_table, delete_state_on_target, complete_delete_state_on_target);
}
