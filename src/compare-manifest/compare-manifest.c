/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2022  Sander van der Burg
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

#include "compare-manifest.h"
#include <manifest.h>

int compare_manifest(const gchar *new_manifest, gchar *old_manifest, const gchar *coordinator_profile_path, gchar *profile)
{
    Manifest *manifest = create_manifest(new_manifest, MANIFEST_ALL_FLAGS, NULL, NULL);

    if(manifest == NULL)
    {
        g_printerr("[coordinator]: Error opening manifest file!\n");
        return 1;
    }
    else
    {
        int exit_status;

        if(check_manifest(manifest))
        {
            gchar *old_manifest_file = determine_manifest_to_open(old_manifest, coordinator_profile_path, profile);
            Manifest *previous_manifest = open_previous_manifest(old_manifest_file, MANIFEST_ALL_FLAGS, NULL, NULL);

            if(previous_manifest == NULL)
                exit_status = 1; /* If no previous manifest exists, then consider the current manifest not equal */
            else
            {
                if(check_manifest(previous_manifest))
                    exit_status = !compare_manifests(manifest, previous_manifest);
                else
                    exit_status = 2;

                delete_manifest(previous_manifest);
            }

            g_free(old_manifest_file);
        }
        else
            exit_status = 2;

        delete_manifest(manifest);

        return exit_status;
    }
}
