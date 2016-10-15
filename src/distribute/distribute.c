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

#include "distribute.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <client-interface.h>
#include <manifest.h>
#include <distributionmapping.h>
#include <targets.h>

static int wait_to_complete(void)
{
    int status;
    pid_t pid = wait(&status);
    
    if(pid == -1 || WEXITSTATUS(status) != 0)
    {
        g_printerr("Cannot transfer intra-dependency closure!\n");
        return 1;
    }
    else
        return 0;
}

int distribute(const gchar *manifest_file, const unsigned int max_concurrent_transfers)
{
    /* Generate a distribution array from the manifest file */
    Manifest *manifest = create_manifest(manifest_file, NULL, NULL);
    
    if(manifest == NULL)
    {
        g_print("[coordinator]: Error while opening manifest file!\n");
        return 1;
    }
    else
    {
        unsigned int i;
        int exit_status;
        unsigned int running_processes = 0;
        
        /* Iterate over the distribution array and distribute the profiles to the target machines */
        for(i = 0; i < manifest->distribution_array->len; i++)
        {
            DistributionItem *item = g_ptr_array_index(manifest->distribution_array, i);
            Target *target = find_target(manifest->target_array, item->target);
            
            /* Invoke copy closure operation */
            g_print("[target: %s]: Receiving intra-dependency closure of profile: %s\n", item->target, item->profile);
            exec_copy_closure_to(target->client_interface, item->target, item->profile);
            running_processes++;
            
            /* If limit has been reached, wait until one of the transfers finishes */
            if(running_processes >= max_concurrent_transfers)
            {
                exit_status = wait_to_complete();
                running_processes--;
                
                if(exit_status != 0)
                    break;
            }
        }
        
        /* Wait for remaining transfers to finish */
        for(i = 0; i < running_processes; i++)
        {
            exit_status = wait_to_complete();
            if(exit_status != 0)
                break;
        }
        
        /* Delete manifest from memory */
        delete_manifest(manifest);
        
        /* Return the exit status, which is 0 if everything succeeds */
        return exit_status;
    }
}
