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

#include "run-set-profiles.h"
#include <manifest.h>

int run_set_profiles(const gchar *manifest_file, const gchar *coordinator_profile_path, char *profile, const unsigned int flags)
{
    Manifest *manifest = create_manifest(manifest_file, MANIFEST_PROFILES_FLAG | MANIFEST_INFRASTRUCTURE_FLAG, NULL, NULL);

    if(manifest == NULL)
    {
        g_printerr("[coordinator]: Error opening manifest file!\n");
        return 1;
    }
    else
    {
        int exit_status;

        if(check_manifest(manifest))
            exit_status = !set_profiles(manifest, manifest_file, coordinator_profile_path, profile, flags);
        else
            exit_status = 1;

        /* Cleanup */
        delete_manifest(manifest);

        /* Return exit status */
        return exit_status;
    }
}
