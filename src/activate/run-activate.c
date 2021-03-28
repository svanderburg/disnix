/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2021  Sander van der Burg
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

#include "run-activate.h"
#include <activate.h>
#include <manifest.h>
#include <interrupt.h>

int run_activate_system(const gchar *new_manifest, const gchar *old_manifest, const gchar *coordinator_profile_path, gchar *profile, const unsigned int flags)
{
    Manifest *manifest = create_manifest(new_manifest, MANIFEST_SERVICE_MAPPINGS_FLAG | MANIFEST_INFRASTRUCTURE_FLAG, NULL, NULL);

    if(manifest == NULL)
    {
        g_printerr("[coordinator]: Error opening manifest file!\n");
        return 1;
    }
    else
    {
        TransitionStatus status;

        if(check_manifest(manifest))
        {
            gchar *old_manifest_file = determine_manifest_to_open(old_manifest, coordinator_profile_path, profile);
            Manifest *previous_manifest = open_previous_manifest(old_manifest_file, MANIFEST_SERVICE_MAPPINGS_FLAG, NULL, NULL);

            /* Do the activation process */
            status = activate_system(manifest, previous_manifest, flags, set_flag_on_interrupt, restore_default_behaviour_on_interrupt);
            print_transition_status(status, old_manifest_file, new_manifest, coordinator_profile_path, profile);

            /* Cleanup */
            delete_manifest(previous_manifest);
            g_free(old_manifest_file);
        }
        else
            status = TRANSITION_FAILED;

        delete_manifest(manifest);

        /* Return the transition status */
        return status;
    }
}
