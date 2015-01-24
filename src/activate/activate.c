/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2013  Sander van der Burg
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
#include "locking.h"
#include "transition.h"
#include "profiles.h"

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

static gchar *determine_previous_manifest_file(const gchar *coordinator_profile_path, const char *username, const gchar *profile)
{
    gchar *old_manifest_file;
    FILE *file;
    
    if(coordinator_profile_path == NULL)
        old_manifest_file = g_strconcat(LOCALSTATEDIR "/nix/profiles/per-user/", username, "/disnix-coordinator/", profile, NULL);
    else
        old_manifest_file = g_strconcat(coordinator_profile_path, "/", profile, NULL);
    
    /* Try to open file => if it succeeds we have a previous configuration */
    file = fopen(old_manifest_file, "r");
    
    if(file == NULL)
    {
        g_free(old_manifest_file);
        old_manifest_file = NULL;
    }
    else
        fclose(file);
    
    return old_manifest_file;
}

static void set_flag_on_interrupt(void)
{
    struct sigaction act;
    act.sa_handler = handle_sigint;
    act.sa_flags = 0;
    
    sigaction(SIGINT, &act, NULL);
}

static void release_locks(const gboolean no_lock, const gboolean dry_run, gchar *interface, GArray *distribution_array, gchar *profile)
{
    if(no_lock || dry_run)
        g_print("[coordinator]: Not releasing any locks, because they have been disabled\n");
    else
    {
        g_print("[coordinator]: Releasing locks on each target\n");
        unlock(interface, distribution_array, profile);
    }
}

static void cleanup(gchar *old_manifest_file, Manifest *manifest, GArray *old_activation_mappings)
{
    g_free(old_manifest_file);
    delete_manifest(manifest);
    delete_activation_array(old_activation_mappings);
}

int activate_system(gchar *interface, const gchar *new_manifest, const gchar *old_manifest, const gchar *coordinator_profile_path, gchar *profile, const gboolean no_coordinator_profile, const gboolean no_target_profiles, const gboolean no_upgrade, const gboolean no_lock, const gboolean dry_run)
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
        GArray *old_activation_mappings;
        
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
        
        /* Try to acquire locks */
        if(no_lock || dry_run)
            g_print("[coordinator]: Not acquiring any locks, because they have been disabled\n");
        else
        {
            g_print("[coordinator]: Acquiring locks on each target\n");
            
            if((status = lock(interface, manifest->distribution_array, profile)) != 0)
            {
                release_locks(no_lock, dry_run, interface, manifest->distribution_array, profile);
                cleanup(old_manifest_file, manifest, old_activation_mappings);
                g_printerr("[coordinator]: ERROR: Lock phase execution failed!\n");
                return status;
            }
        }
        
        /* Execute transition */
        g_print("[coordinator]: Executing the transition to the new deployment state\n");
        
        if((status = transition(interface, manifest->activation_array, old_activation_mappings, manifest->target_array, dry_run)) != 0)
        {
            release_locks(no_lock, dry_run, interface, manifest->distribution_array, profile);
            cleanup(old_manifest_file, manifest, old_activation_mappings);
            g_printerr("[coordinator]: ERROR: Transition phase execution failed!\n");
            return status;
        }
        
        /* Set the new profiles on the target machines */
        
        if(no_target_profiles || dry_run)
            g_print("[coordinator]: Setting target profiles has been disabled\n");
        else
        {
            g_print("[coordinator]: Setting the profiles on the target machines\n");
        
            if((status = set_target_profiles(manifest->distribution_array, interface, profile)) != 0)
            {
                release_locks(no_lock, dry_run, interface, manifest->distribution_array, profile);
                cleanup(old_manifest_file, manifest, old_activation_mappings);
                g_printerr("[coordinator]: ERROR: Cannot set profiles on the target machines!\n");
                return status;
            }
        }
        
        /* Release the locks */
        release_locks(no_lock, dry_run, interface, manifest->distribution_array, profile);
        
        /* Set the coordinator profile */
        if(no_coordinator_profile || dry_run)
            g_print("[coordinator]: Not setting the coordinator profile\n");
        else
        {
            g_print("[coordinator]: Setting the coordinator profile: %s\n", profile);
            
            if((status = set_coordinator_profile(coordinator_profile_path, new_manifest, profile, username)) != 0)
            {
                cleanup(old_manifest_file, manifest, old_activation_mappings);
                g_printerr("[coordinator]: ERROR: Cannot update the coordinator profile!\n");
                return status;
            }
        }
        
        /* Cleanup */
        cleanup(old_manifest_file, manifest, old_activation_mappings);
        
        /* Everything succeeded */
        g_printerr("[coordinator]: The new configuration has been successfully activated!\n");
        return 0;
    }
}
