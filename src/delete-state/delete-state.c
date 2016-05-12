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

#include "delete-state.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <client-interface.h>
#include <manifest.h>
#include <snapshotmapping.h>
#include <targets.h>

static int wait_to_complete_delete(GHashTable *pids, GPtrArray *target_array)
{
    int status;
    pid_t pid = wait(&status);
    
    if(pid == -1)
        return FALSE;
    else
    {
        Target *target;
        
        /* Find the corresponding snapshot mapping and remove it from the pids table */
        SnapshotMapping *mapping = g_hash_table_lookup(pids, &pid);
        g_hash_table_remove(pids, &pid);
        
        /* Mark mapping as transferred to prevent it from deleting again */
        mapping->transferred = TRUE;
        
        /* Signal the target to make the CPU core available again */
        target = find_target(target_array, mapping->target);
        signal_available_target_core(target);
        
        /* Return the status */
        if(WEXITSTATUS(status) == 0)
            return TRUE;
        else
        {
            g_printerr("Cannot delete state!\n");
            return FALSE;
        }
    }
}

static void destroy_pids_key(gpointer data)
{
    gint *key = (gint*)data;
    g_free(key);
}

static int delete_obsolete_state(GPtrArray *snapshots_array, GPtrArray *target_array)
{
    unsigned int num_deleted = 0;
    int status = TRUE;
    GHashTable *pids = g_hash_table_new_full(g_int_hash, g_int_equal, destroy_pids_key, NULL);
    
    while(num_deleted < snapshots_array->len)
    {
        unsigned int i;
    
        for(i = 0; i < snapshots_array->len; i++)
        {
            SnapshotMapping *mapping = g_ptr_array_index(snapshots_array, i);
            Target *target = find_target(target_array, mapping->target);
            
            if(!mapping->transferred && request_available_target_core(target)) /* Check if machine has any cores available, if not wait and try again later */
            {
                gchar **arguments = generate_activation_arguments(target, mapping->container); /* Generate an array of key=value pairs from container properties */
                unsigned int arguments_size = g_strv_length(arguments); /* Determine length of the activation arguments array */
                pid_t pid;
                gint *pidKey;
                
                g_print("[target: %s]: Deleting obsolete state of service: %s\n", mapping->target, mapping->component);
                pid = exec_delete_state(target->client_interface, mapping->target, mapping->container, mapping->type, arguments, arguments_size, mapping->service);
                
                /* Add pid and mapping to the hash table */
                pidKey = g_malloc(sizeof(gint));
                *pidKey = pid;
                g_hash_table_insert(pids, pidKey, mapping);
            
                /* Cleanup */
                g_strfreev(arguments);
            }
        }
    
        if(!wait_to_complete_delete(pids, target_array))
            status = FALSE;
        
        num_deleted++;
    }
    
    g_hash_table_destroy(pids);
    return status;
}

int delete_state(const gchar *manifest_file, const gchar *coordinator_profile_path, gchar *profile)
{
    Manifest *manifest;
    
    if(manifest_file == NULL)
    {
        /* Get current username */
        char *username = (getpwuid(geteuid()))->pw_name;
        
        /* If no manifest file has been provided, try opening the last deployed one */
        gchar *old_manifest_file = determine_previous_manifest_file(coordinator_profile_path, username, profile);
        
        if(old_manifest_file == NULL)
        {
            g_printerr("[coordinator]: No previous manifest file exists, so no state will be deleted!\n");
            return 0;
        }
        else
        {
            g_printerr("[coordinator]: Deleting obsolete state of all components of the previous generation: %s\n", old_manifest_file);
            manifest = create_manifest(old_manifest_file);
            g_free(old_manifest_file);
        }
    }
    else
    {
        /* Open the provided manifest */
        g_printerr("[coordinator]: Deleting obsolete state of all components...\n");
        manifest = create_manifest(manifest_file);
    }
    
    if(manifest == NULL)
    {
        g_print("[coordinator]: Error while opening manifest file!\n");
        return 1;
    }
    else
    {
        /* Delete the obsolete state */
        int exit_status;
        
        if(delete_obsolete_state(manifest->snapshots_array, manifest->target_array))
            exit_status = 0;
        else
            exit_status = 1;
        
        delete_manifest(manifest);
        return exit_status;
    }
}
