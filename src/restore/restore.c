/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2015  Sander van der Burg
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

#include "restore.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <stdio.h>
#include <client-interface.h>
#include <manifest.h>
#include <snapshotmapping.h>
#include <targets.h>

static int wait_to_complete_retrieve(void)
{
    int status;
    pid_t pid = wait(&status);
    
    if(pid == -1 || WEXITSTATUS(status) != 0)
    {
        g_printerr("Cannot send snapshots!\n");
        return 1;
    }
    else
        return 0;
}

static int send_snapshots(GPtrArray *snapshots_array, GPtrArray *target_array, const unsigned int max_concurrent_transfers, const int all)
{
    unsigned int running_processes = 0, i;
    int exit_status;
    
    for(i = 0; i < snapshots_array->len; i++)
    {
        SnapshotMapping *mapping = g_ptr_array_index(snapshots_array, i);
        Target *target = find_target(target_array, mapping->target);
        gchar *interface = find_target_client_interface(target);
        
        g_print("[target: %s]: Sending snapshots of component: %s deployed to container: %s\n", mapping->target, mapping->component, mapping->container);
        exec_copy_snapshots_to(interface, mapping->target, mapping->container, mapping->component, all);
        running_processes++;
        
        /* If limit has been reached, wait until one of the transfers finishes */
        if(running_processes >= max_concurrent_transfers)
        {
            exit_status = wait_to_complete_retrieve();
            running_processes--;
            
            if(exit_status != 0)
                break;
        }
    }
    
    /* Wait for remaining transfers to finish */
    for(i = 0; i < running_processes; i++)
    {
        exit_status = wait_to_complete_retrieve();
        if(exit_status != 0)
            break;
    }
    
    return exit_status;
}

static int wait_to_complete_restore(GHashTable *pids, GPtrArray *target_array)
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
        
        /* Mark mapping as transferred to prevent it from restoring again */
        mapping->transferred = TRUE;
        
        /* Signal the target to make the CPU core available again */
        target = find_target(target_array, mapping->target);
        signal_available_target_core(target);
        
        /* Return the status */
        if(WEXITSTATUS(status) == 0)
            return TRUE;
        else
        {
            g_printerr("Cannot restore state!\n");
            return FALSE;
        }
    }
}

static void destroy_pids_key(gpointer data)
{
    gint *key = (gint*)data;
    g_free(key);
}

static int restore_services(GPtrArray *snapshots_array, GPtrArray *target_array)
{
    unsigned int num_restored = 0;
    int status = TRUE;
    GHashTable *pids = g_hash_table_new_full(g_int_hash, g_int_equal, destroy_pids_key, NULL);
    
    while(num_restored < snapshots_array->len)
    {
        unsigned int i;
    
        for(i = 0; i < snapshots_array->len; i++)
        {
            SnapshotMapping *mapping = g_ptr_array_index(snapshots_array, i);
            Target *target = find_target(target_array, mapping->target);
            
            if(!mapping->transferred && request_available_target_core(target)) /* Check if machine has any cores available, if not wait and try again later */
            {
                gchar *interface = find_target_client_interface(target);
                gchar **arguments = generate_activation_arguments(target); /* Generate an array of key=value pairs from infrastructure properties */
                unsigned int arguments_size = g_strv_length(arguments); /* Determine length of the activation arguments array */
                pid_t pid;
                gint *pidKey;
                
                g_print("[target: %s]: Restoring state of service: %s\n", mapping->target, mapping->component);
                pid = exec_restore(interface, mapping->target, mapping->container, arguments, arguments_size, mapping->service);
                
                /* Add pid and mapping to the hash table */
                pidKey = g_malloc(sizeof(gint));
                *pidKey = pid;
                g_hash_table_insert(pids, pidKey, mapping);
            
                /* Cleanup */
                g_strfreev(arguments);
            }
        }
    
        if(!wait_to_complete_restore(pids, target_array))
            status = FALSE;
        
        num_restored++;
    }
    
    g_hash_table_destroy(pids);
    return status;
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

static void cleanup(const gchar *old_manifest, char *old_manifest_file, Manifest *manifest, GPtrArray *snapshots_array)
{
    g_free(old_manifest_file);
    
    if(old_manifest != NULL)
        g_ptr_array_free(snapshots_array, TRUE);
    
    delete_manifest(manifest);
}

int restore(const gchar *manifest_file, const unsigned int max_concurrent_transfers, const int transfer_only, const int all, const gchar *old_manifest, const gchar *coordinator_profile_path, gchar *profile, const gboolean no_upgrade)
{
    /* Generate a distribution array from the manifest file */
    Manifest *manifest = create_manifest(manifest_file);
    
    if(manifest == NULL)
    {
        g_print("[coordinator]: Error while opening manifest file!\n");
        return 1;
    }
    else
    {
        int exit_status = 0;
        GPtrArray *snapshots_array;
        gchar *old_manifest_file;
        
        /* Get current username */
        char *username = (getpwuid(geteuid()))->pw_name;
        
        if(old_manifest == NULL)
            old_manifest_file = determine_previous_manifest_file(coordinator_profile_path, username, profile);
        else
            old_manifest_file = g_strdup(old_manifest);
        
        if(no_upgrade || old_manifest_file == NULL)
        {
            g_printerr("[coordinator]: Sending snapshots of all components...\n");
            snapshots_array = manifest->snapshots_array;
        }
        else
        {
            GPtrArray *old_snapshots_array = create_snapshots_array(old_manifest_file);
            g_printerr("[coordinator]: Snapshotting state of moved components...\n");
            snapshots_array = subtract_snapshot_mappings(manifest->snapshots_array, old_snapshots_array);
            delete_snapshots_array(old_snapshots_array);
        }
        
        if((exit_status = send_snapshots(snapshots_array, manifest->target_array, max_concurrent_transfers, all)) != 0)
        {
            cleanup(old_manifest, old_manifest_file, manifest, snapshots_array);
            return exit_status;
        }
        
        g_print("[coordinator]: Restoring state of services...\n");
        
        if(!transfer_only && !restore_services(snapshots_array, manifest->target_array))
        {
            cleanup(old_manifest, old_manifest_file, manifest, snapshots_array);
            return 1;
        }
        
        cleanup(old_manifest, old_manifest_file, manifest, snapshots_array);
        return exit_status;
    }
}