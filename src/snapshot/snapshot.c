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

#include "snapshot.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <client-interface.h>
#include <manifest.h>
#include <activationmapping.h>
#include <targets.h>

static int wait_to_complete_snapshot(GHashTable *pids, GPtrArray *target_array)
{
    int status;
    pid_t pid = wait(&status);
    
    if(pid == -1)
        return FALSE;
    else
    {
        Target *target;
        
        /* Find the corresponding activation mapping and remove it from the pids table */
        ActivationMapping *mapping = g_hash_table_lookup(pids, &pid);
        g_hash_table_remove(pids, &pid);
        
        /* Mark mapping as activated to prevent it from snapshotting again */
        mapping->status = ACTIVATIONMAPPING_ACTIVATED;
        
        /* Signal the target to make the CPU core available again */
        target = find_target(target_array, mapping->target);
        signal_available_target_core(target);
        
        /* Return the status */
        if(WEXITSTATUS(status) == 0)
            return TRUE;
        else
        {
            g_printerr("Cannot snapshot state!\n");
            return FALSE;
        }
    }
}

static void destroy_pids_key(gpointer data)
{
    gint *key = (gint*)data;
    g_free(key);
}

static void mark_deactivated(GPtrArray *activation_array)
{
    unsigned int i;
    
    for(i = 0; i < activation_array->len; i++)
    {
        ActivationMapping *mapping = g_ptr_array_index(activation_array, i);
        mapping->status = ACTIVATIONMAPPING_DEACTIVATED;
    }
}

static int snapshot_services(GPtrArray *activation_array, GPtrArray *target_array)
{
    unsigned int num_snapshotted = 0;
    int status = TRUE;
    GHashTable *pids = g_hash_table_new_full(g_int_hash, g_int_equal, destroy_pids_key, NULL);
    
    mark_deactivated(activation_array);
    
    while(num_snapshotted < activation_array->len)
    {
        unsigned int i;
    
        for(i = 0; i < activation_array->len; i++)
        {
            ActivationMapping *mapping = g_ptr_array_index(activation_array, i);
            Target *target = find_target(target_array, mapping->target);
            
            if(mapping->status == ACTIVATIONMAPPING_DEACTIVATED && request_available_target_core(target)) /* Check if machine has any cores available, if not wait and try again later */
            {
                gchar *interface = find_target_client_interface(target);
                gchar **arguments = generate_activation_arguments(target); /* Generate an array of key=value pairs from infrastructure properties */
                unsigned int arguments_size = g_strv_length(arguments); /* Determine length of the activation arguments array */
                pid_t pid;
                gint *pidKey;
                
                g_print("[target: %s]: Snapshotting state of service: %s\n", mapping->target, mapping->service);
                pid = exec_snapshot(interface, mapping->target, mapping->type, arguments, arguments_size, mapping->service);
                
                /* Add pid and mapping to the hash table */
                pidKey = g_malloc(sizeof(gint));
                *pidKey = pid;
                g_hash_table_insert(pids, pidKey, mapping);
            
                /* Cleanup */
                g_strfreev(arguments);
            }
        }
    
        if(!wait_to_complete_snapshot(pids, target_array))
            status = FALSE;
        
        num_snapshotted++;
    }
    
    g_hash_table_destroy(pids);
    return status;
}

static int wait_to_complete_retrieve(void)
{
    int status;
    pid_t pid = wait(&status);
    
    if(pid == -1 || WEXITSTATUS(status) != 0)
    {
        g_printerr("Cannot retrieve snapshots!\n");
        return 1;
    }
    else
        return 0;
}

static int retrieve_snapshots(GPtrArray *activation_array, GPtrArray *target_array, const unsigned int max_concurrent_transfers, const int all)
{
    unsigned int running_processes = 0, i;
    int exit_status;
    
    for(i = 0; i < activation_array->len; i++)
    {
        ActivationMapping *mapping = g_ptr_array_index(activation_array, i);
        Target *target = find_target(target_array, mapping->target);
        gchar *interface = find_target_client_interface(target);
        
        g_print("[target: %s]: Retrieving snapshots of component: %s deployed to container: %s\n", mapping->target, mapping->service, mapping->type);
        exec_copy_snapshots_from(interface, mapping->target, mapping->type, mapping->service, all);
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

int snapshot(const gchar *manifest_file, const unsigned int max_concurrent_transfers, const int transfer_only, const int all)
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
        
        g_print("[coordinator]: Snapshotting state of services...\n");
        
        if(!transfer_only && !snapshot_services(manifest->activation_array, manifest->target_array))
        {
            delete_manifest(manifest);
            return 1;
        }
        
        g_print("[coordinator]: Retrieving snapshots...\n");
        
        if((exit_status = retrieve_snapshots(manifest->activation_array, manifest->target_array, max_concurrent_transfers, all)) != 0)
        {
            delete_manifest(manifest);
            return exit_status;
        }
        
        delete_manifest(manifest);
        return exit_status;
    }
}
