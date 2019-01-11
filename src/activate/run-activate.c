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

#include "run-activate.h"
#include <activate.h>
#include <manifest.h>
#include <activationmapping.h>
#include <interrupt.h>

int run_activate_system(const gchar *new_manifest, const gchar *old_manifest, const gchar *coordinator_profile_path, gchar *profile, const unsigned int flags)
{
    Manifest *manifest = create_manifest(new_manifest, MANIFEST_ACTIVATION_FLAG, NULL, NULL);

    if(manifest == NULL)
    {
        g_printerr("[coordinator]: Error opening manifest file!\n");
        return 1;
    }
    else
    {
        TransitionStatus status;
        gchar *old_manifest_file;
        GPtrArray *old_activation_mappings;

        /* If no previous configuration is given, check whether we have one in the coordinator profile, otherwise use the given one */
        if(old_manifest == NULL)
            old_manifest_file = determine_previous_manifest_file(coordinator_profile_path, profile);
        else
            old_manifest_file = g_strdup(old_manifest);

        /* If we have an old configuration -> open it */
        if(old_manifest_file == NULL)
            old_activation_mappings = NULL;
        else
            old_activation_mappings = create_activation_array(old_manifest_file);

        /* Override SIGINT's behaviour to allow stuff to be rollbacked in case of an interruption */
        set_flag_on_interrupt();

        /* Do the activation process */
        status = activate_system(old_manifest_file, new_manifest, manifest, old_activation_mappings, coordinator_profile_path, profile, flags);

        /* Cleanup */
        g_free(old_manifest_file);
        delete_manifest(manifest);
        delete_activation_array(old_activation_mappings);

        /* Return the transition status */
        return status;
    }
}
