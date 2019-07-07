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

#include "run-delete-state.h"
#include "delete-state.h"
#include <client-interface.h>
#include <manifest.h>
#include <snapshotmappingarray.h>
#include <targets.h>

int run_delete_state(const gchar *manifest_file, const gchar *coordinator_profile_path, gchar *profile, const gchar *container, const gchar *component)
{
    Manifest *manifest = open_provided_or_previous_manifest_file(manifest_file, coordinator_profile_path, profile, MANIFEST_SNAPSHOT_FLAG | MANIFEST_TARGETS_FLAG, container, component);

    if(manifest == NULL)
    {
        g_printerr("[coordinator]: Error while opening manifest file!\n");
        g_printerr("Please provde a valid manifest file as a command-line parameter.\n");
        return 1;
    }
    else
    {
        int exit_status;

        if(check_manifest(manifest))
        {
            g_printerr("[coordinator]: Deleting obsolete state of services...\n");
            exit_status = !delete_obsolete_state(manifest->snapshot_mapping_array, manifest->services_table, manifest->targets_table);
        }
        else
            exit_status = 1;

        delete_manifest(manifest);
        return exit_status;
    }
}
