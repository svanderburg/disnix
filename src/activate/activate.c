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

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdio.h>
#include <signal.h>

#include <manifest.h>
#include <activationmapping.h>

volatile int interrupted = FALSE;

static void handle_sigint(int signum)
{
    interrupted = TRUE;
}

static void set_flag_on_interrupt(void)
{
    struct sigaction act;
    act.sa_handler = handle_sigint;
    act.sa_flags = 0;
    
    sigaction(SIGINT, &act, NULL);
}

static void cleanup(gchar *old_manifest_file, Manifest *manifest, GPtrArray *old_activation_mappings)
{
    g_free(old_manifest_file);
    delete_manifest(manifest);
    delete_activation_array(old_activation_mappings);
}

int activate_system(const gchar *new_manifest, const gchar *old_manifest, const gchar *coordinator_profile_path, gchar *profile, const gboolean no_upgrade, const gboolean no_rollback, const gboolean dry_run)
{
    Manifest *manifest = create_manifest(new_manifest);
    
    if(manifest == NULL)
    {
        g_printerr("[coordinator]: Error opening manifest_file!\n");
        return 1;
    }
    else
    {
        int status;
        gchar *old_manifest_file;
        GPtrArray *old_activation_mappings;
        
        /* Get current username */
        char *username = (getpwuid(geteuid()))->pw_name;

        /* If no previous configuration is given, check whether we have one in the coordinator profile, otherwise use the given one */
        if(old_manifest == NULL)
            old_manifest_file = determine_previous_manifest_file(coordinator_profile_path, username, profile);
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
        
        if((status = transition(manifest->activation_array, old_activation_mappings, manifest->target_array, no_rollback, dry_run)) != 0)
        {
            cleanup(old_manifest_file, manifest, old_activation_mappings);
            g_printerr("[coordinator]: ERROR: Transition phase execution failed!\n");
            
            if(old_manifest_file != NULL && status == ROLLBACK_FAILED)
            {
                g_printerr("The rollback failed! This means the system is now inconsistent! Please\n");
                g_printerr("diagnose the errors before doing another redeployment!\n\n");
                
                g_printerr("When the problems have been solved, the rollback can be triggered again, by\n");
                g_printerr("running:\n\n");
                g_printerr("$ disnix-activate --no-rollback -p %s --coordinator-profile-path %s -o %s %s\n\n", profile, coordinator_profile_path, new_manifest, old_manifest_file);
            }
            
            return status;
        }
        
        /* Cleanup */
        cleanup(old_manifest_file, manifest, old_activation_mappings);
        
        /* Everything succeeded */
        g_printerr("[coordinator]: The new configuration has been successfully activated!\n");
        return 0;
    }
}
