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

#include "activate.h"
#include <servicemappingarray.h>

void print_transition_status(TransitionStatus status, const gchar *old_manifest_file, const gchar *new_manifest_file, const gchar *coordinator_profile_path, const gchar *profile)
{
    if(status == TRANSITION_SUCCESS)
        g_printerr("[coordinator]: The new configuration has been successfully activated!\n");
    else
    {
        g_printerr("[coordinator]: ERROR: Transition phase execution failed!\n");

        if(old_manifest_file != NULL)
        {
            if(status == TRANSITION_NEW_MAPPINGS_ROLLBACK_FAILED)
            {
                g_printerr("\nThe new mappings rollback failed! This means the system is now inconsistent!\n");
                g_printerr("Please manually diagnose the errors before doing another redeployment!\n\n");

                g_printerr("When the problems have been solved, the rollback can be triggered again, by\n");
                g_printerr("running:\n\n");

                g_printerr("$ disnix-activate --no-rollback -p %s ", profile);

                if(coordinator_profile_path != NULL)
                    g_printerr("--coordinator-profile-path %s ", coordinator_profile_path);

                g_printerr("-o %s %s\n\n", new_manifest_file, old_manifest_file);
            }
            else if(status == TRANSITION_OBSOLETE_MAPPINGS_ROLLBACK_FAILED)
            {
                g_printerr("\nThe obsolete mappings rollback failed! This means the system is now\n");
                g_printerr("inconsistent! Please manually diagnose the errors before doing another\n");
                g_printerr("redeployment!\n\n");

                g_printerr("When the problems have been solved, the rollback can be triggered again, by\n");
                g_printerr("running:\n\n");

                g_printerr("$ disnix-activate --no-upgrade --no-rollback -p %s ", profile);

                if(coordinator_profile_path != NULL)
                    g_printerr("--coordinator-profile-path %s ", coordinator_profile_path);

                g_printerr("%s\n\n", old_manifest_file);
            }
        }
    }
}

TransitionStatus activate_system(Manifest *manifest, Manifest *previous_manifest, const unsigned int flags, void (*pre_hook) (void), void (*post_hook) (void))
{
    TransitionStatus status;

    /* Execute transition */
    g_print("[coordinator]: Executing the transition to the new deployment state\n");

    if(flags & FLAG_NO_UPGRADE)
    {
        g_print("[coordinator]: Forced to do no upgrade! Ignoring the previous manifest file!\n");
        previous_manifest = NULL;
    }
    else if(previous_manifest != NULL)
        g_print("[coordinator]: Doing an upgrade from previous manifest file\n");
    else
        g_print("[coordinator]: Doing an installation from scratch\n");

    if(pre_hook != NULL) /* Execute hook before the lock operations are executed */
        pre_hook();

    status = transition(manifest, previous_manifest, flags);

    if(post_hook != NULL) /* Execute hook after the lock operations have been completed */
        post_hook();

    return status;
}
