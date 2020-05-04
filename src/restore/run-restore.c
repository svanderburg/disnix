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

#include "run-restore.h"
#include <manifest.h>
#include <snapshotmappingarray.h>

int run_restore(const gchar *manifest_file, const unsigned int max_concurrent_transfers, const unsigned int flags, const int keep, const gchar *old_manifest, const gchar *coordinator_profile_path, gchar *profile, const gchar *container_filter, const gchar *component_filter)
{
    /* Generate a distribution array from the manifest file */
    Manifest *manifest = open_provided_or_previous_manifest_file(manifest_file, coordinator_profile_path, profile, MANIFEST_SNAPSHOT_MAPPINGS_FLAG | MANIFEST_INFRASTRUCTURE_FLAG, container_filter, component_filter);

    if(manifest == NULL)
    {
        g_print("[coordinator]: Error while opening manifest file!\n");
        return 1;
    }
    else
    {
        int exit_status;

        if(check_manifest(manifest))
        {
            Manifest *previous_manifest;

            if(flags & FLAG_NO_UPGRADE)
                previous_manifest = NULL;
            else
                previous_manifest = open_provided_or_previous_manifest_file(old_manifest, coordinator_profile_path, profile, MANIFEST_SNAPSHOT_MAPPINGS_FLAG, container_filter, component_filter);

            if(previous_manifest == NULL || check_manifest(previous_manifest))
                exit_status = !restore(manifest, previous_manifest, max_concurrent_transfers, flags, keep);
            else
                exit_status = 1;

            /* Cleanup */
            delete_manifest(previous_manifest);
        }
        else
            exit_status = 1;

        delete_manifest(manifest);

        /* Return the exit status */
        return exit_status;
    }
}
