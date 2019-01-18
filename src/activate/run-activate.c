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
        gchar *old_manifest_file = determine_manifest_to_open(old_manifest, coordinator_profile_path, profile);
        GPtrArray *old_activation_mappings = open_previous_activation_array(old_manifest_file);

        /* Override SIGINT's behaviour to allow stuff to be rollbacked in case of an interruption */
        set_flag_on_interrupt();

        /* Do the activation process */
        status = activate_system(manifest, old_activation_mappings, flags);
        print_transition_status(status, old_manifest_file, new_manifest, coordinator_profile_path, profile);

        /* Cleanup */
        delete_activation_array(old_activation_mappings);
        g_free(old_manifest_file);
        delete_manifest(manifest);

        /* Return the transition status */
        return status;
    }
}
