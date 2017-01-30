/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2016  Sander van der Burg
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
#include "transition.h"
#include <manifest.h>
#include <activationmapping.h>
#include <interrupt.h>

int activate_system(const gchar *new_manifest, const gchar *old_manifest, const gchar *coordinator_profile_path, gchar *profile, const gboolean no_upgrade, const gboolean no_rollback, const gboolean dry_run)
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
        if(!no_upgrade && old_manifest_file != NULL)
        {
            g_print("[coordinator]: Doing an upgrade from previous manifest file: %s\n", old_manifest_file);
            old_activation_mappings = create_activation_array(old_manifest_file);
        }
        else
        {
            g_print("[coordinator]: Doing an installation from scratch\n");
            old_activation_mappings = NULL;
        }

        /* Override SIGINT's behaviour to allow stuff to be rollbacked in case of an interruption */
        set_flag_on_interrupt();
        
        /* Execute transition */
        g_print("[coordinator]: Executing the transition to the new deployment state\n");
        
        if((status = transition(manifest->activation_array, old_activation_mappings, manifest->target_array, no_rollback, dry_run)) == TRANSITION_SUCCESS)
            g_printerr("[coordinator]: The new configuration has been successfully activated!\n");
        else
        {
            g_printerr("[coordinator]: ERROR: Transition phase execution failed!\n");
            
            if(old_manifest_file != NULL)
            {
                if(status == TRANSITION_NEW_MAPPINGS_ROLLBACK_FAILED)
                {
                    g_printerr("The new mappings rollback failed! This means the system is now inconsistent!\n");
                    g_printerr("Please manually diagnose the errors before doing another redeployment!\n\n");
                    
                    g_printerr("When the problems have been solved, the rollback can be triggered again, by\n");
                    g_printerr("running:\n\n");
                    g_printerr("$ disnix-activate --no-rollback -p %s ", profile);
                    
                    if(coordinator_profile_path != NULL)
                        g_printerr("--coordinator-profile-path %s ", coordinator_profile_path);
                    
                    g_printerr("-o %s %s\n\n", new_manifest, old_manifest_file);
                }
                else if(status == TRANSITION_OBSOLETE_MAPPINGS_ROLLBACK_FAILED)
                {
                    g_printerr("The obsolete mappings rollback failed! This means the system is now\n");
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
        
        /* Cleanup */
        g_free(old_manifest_file);
        delete_manifest(manifest);
        delete_activation_array(old_activation_mappings);

        /* Return the transition status */
        return status;
    }
}
