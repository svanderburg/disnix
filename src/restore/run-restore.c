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

#include "run-restore.h"
#include <manifest.h>
#include <snapshotmapping.h>

int run_restore(const gchar *manifest_file, const unsigned int max_concurrent_transfers, const unsigned int flags, const int keep, const gchar *old_manifest, const gchar *coordinator_profile_path, gchar *profile, const gchar *container_filter, const gchar *component_filter)
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
        int exit_status;
        GPtrArray *old_snapshots_array;

        if(flags & FLAG_NO_UPGRADE)
            old_snapshots_array = NULL;
        else
            old_snapshots_array = open_provided_or_previous_snapshots_array(old_manifest, coordinator_profile_path, profile, container_filter, component_filter);

        exit_status = !restore(manifest, old_snapshots_array, max_concurrent_transfers, flags, keep);

        /* Cleanup */
        delete_snapshots_array(old_snapshots_array);
        delete_manifest(manifest);

        /* Return the exit status */
        return exit_status;
    }
}
