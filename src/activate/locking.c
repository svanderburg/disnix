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

#include "locking.h"
#include <distributionmapping.h>
#include <client-interface.h>

extern volatile int interrupted;

int unlock(gchar *interface, GArray *distribution_array, gchar *profile)
{
    unsigned int i, running_processes = 0;
    int exit_status = 0;
    int status;
    
    /* For each locked machine, release the lock */
    for(i = 0; i < distribution_array->len; i++)
    {
        DistributionItem *item = g_array_index(distribution_array, DistributionItem*, i);
        
        g_print("[target: %s]: Releasing a lock!\n", item->target);
        status = exec_unlock(interface, item->target, profile);
        
        if(status == -1)
        {
            g_printerr("[target: %s]: Error with forking unlock process!\n", item->target);
            exit_status = -1;
        }
        else
            running_processes++;
    }
    
    /* Wait until every lock is released */
    for(i = 0; i < running_processes; i++)
    {
        status = wait_to_finish(0);
        
        /* If a process fails, change the exit status */
        if(status != 0)
        {
            g_printerr("Failed to release the lock!\n");
            exit_status = status;
        }
    }
    
    /* Return exit status, which is 0 if everything succeeds */
    return exit_status;
}

int lock(gchar *interface, GArray *distribution_array, gchar *profile)
{
    unsigned int i;
    GArray *try_array = g_array_new(FALSE, FALSE, sizeof(DistributionItem*));
    GArray *lock_array = g_array_new(FALSE, FALSE, sizeof(DistributionItem*));
    int exit_status = 0;
    int status;
    
    /* For each machine acquire a lock */
    for(i = 0; i < distribution_array->len; i++)
    {
        DistributionItem *item = g_array_index(distribution_array, DistributionItem*, i);
        
        g_print("[target: %s]: Acquiring a lock\n", item->target);
        status = exec_lock(interface, item->target, profile);
        
        /* If a process fails, change the exit status */
        if(status == -1)
        {
            g_printerr("[target: %s]: Error with forking lock process!\n", item->target);
            exit_status = -1;
        }
        else
            g_array_append_val(try_array, item);
    }
    
    /* Wait until every lock is acquired */
    for(i = 0; i < try_array->len; i++)
    {
        status = wait_to_finish(0);
        
        /* If a process fails, change the exit status */
        if(status != 0)
        {
            g_printerr("Failed to acquire a lock!\n");
            exit_status = status;
        }
        else if(interrupted)
        {
            g_printerr("The lock phase has been interrupted! Releasing the locks...\n");
            exit_status = 1;
        }
    }
    
    /* If a lock fails then unlock every machine that is locked */
    if(!exit_status)
        unlock(interface, lock_array, profile);
    
    /* Cleanup */
    g_array_free(try_array, TRUE);
    g_array_free(lock_array, TRUE);
    
    /* Return exit status, which is 0 if everything succeeds */
    return exit_status;
}
