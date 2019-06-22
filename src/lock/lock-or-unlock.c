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

#include "lock-or-unlock.h"
#include <locking.h>
#include <manifest.h>
#include <interrupt.h>

/* The entire lock or unlock operation */

int lock_or_unlock(const int do_lock, const gchar *manifest_file, const gchar *coordinator_profile_path, gchar *profile)
{
    Manifest *manifest = open_provided_or_previous_manifest_file(manifest_file, coordinator_profile_path, profile, MANIFEST_DISTRIBUTION_FLAG | MANIFEST_TARGETS_FLAG, NULL, NULL);

    if(manifest == NULL)
    {
        g_printerr("Cannot open any manifest file!\n");
        g_printerr("Please provide a valid manifest as command-line parameter!\n");
        return 1;
    }
    else
    {
        int exit_status;

        if(check_manifest(manifest))
        {
            /* Do the locking */
            if(do_lock)
                exit_status = !lock(manifest->distribution_table, manifest->targets_table, profile, set_flag_on_interrupt, restore_default_behaviour_on_interrupt);
            else
                exit_status = !unlock(manifest->distribution_table, manifest->targets_table, profile, set_flag_on_interrupt, restore_default_behaviour_on_interrupt);
        }
        else
            exit_status = 1;

        /* Cleanup */
        delete_manifest(manifest);

        /* Return exit status */
        return exit_status;
    }
}
