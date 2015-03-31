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

static int snapshot_services(GPtrArray *activation_array, GPtrArray *target_array)
{
    unsigned int numSnapshotted = 0;
    int status = TRUE;
    GHashTable *pids = g_hash_table_new_full(g_int_hash, g_int_equal, destroy_pids_key, NULL);
    
    while(numSnapshotted < activation_array->len)
    {
        unsigned int i;
    
        for(i = 0; i < activation_array->len; i++)
        {
            ActivationMapping *mapping = g_ptr_array_index(activation_array, i);
            Target *target = find_target(target_array, mapping->target);
            
            if(request_available_target_core(target)) /* Check if machine has any cores available, if not wait and try again later */
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
        
        numSnapshotted++;
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

static int retrieve_snapshots(GPtrArray *activation_array, GPtrArray *target_array, const unsigned int max_concurrent_transfers)
{
    unsigned int running_processes = 0, i;
    int exit_status;
    
    for(i = 0; i < activation_array->len; i++)
    {
        ActivationMapping *mapping = g_ptr_array_index(activation_array, i);
        Target *target = find_target(target_array, mapping->target);
        gchar *interface = find_target_client_interface(target);
        
        exec_copy_snapshots_from(interface, mapping->target, mapping->type, mapping->service);
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

int snapshot(const gchar *manifest_file, const unsigned int max_concurrent_transfers)
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
        
        if((exit_status = snapshot_services(manifest->activation_array, manifest->target_array)) != 0)
        {
            delete_manifest(manifest);
            return exit_status;
        }
        
        if((exit_status = retrieve_snapshots(manifest->activation_array, manifest->target_array, max_concurrent_transfers)) != 0)
        {
            delete_manifest(manifest);
            return exit_status;
        }
        
        delete_manifest(manifest);
        return exit_status;
    }
}
