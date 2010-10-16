/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2010  Sander van der Burg
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

gboolean unlock(gchar *interface, GArray *distribution_array, gchar *profile)
{
    unsigned int i, running_processes = 0;
    int exit_status = TRUE;
    int status;
    
    /* For each locked machine, release the lock */
    for(i = 0; i < distribution_array->len; i++)
    {
	DistributionItem *item = g_array_index(distribution_array, DistributionItem*, i);
	
	status = exec_unlock(interface, item->target, profile);
	
	if(status == -1)
	{
	    g_printerr("Error with forking unlock process!\n");
	    exit_status = FALSE;
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
	    exit_status = FALSE;
	}
    }
    
    /* Return exit status */
    return exit_status;
}

gboolean lock(gchar *interface, GArray *distribution_array, gchar *profile)
{
    unsigned int i;
    GArray *try_array = g_array_new(FALSE, FALSE, sizeof(DistributionItem*));
    GArray *lock_array = g_array_new(FALSE, FALSE, sizeof(DistributionItem*));
    int exit_status = TRUE;
    int status;
    
    /* For each machine acquire a lock */
    for(i = 0; i < distribution_array->len; i++)
    {
	DistributionItem *item = g_array_index(distribution_array, DistributionItem*, i);
	
	status = exec_lock(interface, item->target, profile);
	
	/* If a process fails, change the exit status */
	if(status == -1)
	{
	    g_printerr("Error with forking lock process!\n");
	    exit_status = FALSE;
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
	    exit_status = FALSE;
	}
    }
    
    /* If a lock fails then unlock every machine that is locked */
    if(!exit_status)
	unlock(interface, lock_array, profile);
    
    /* Cleanup */
    g_array_free(try_array, TRUE);
    g_array_free(lock_array, TRUE);
    
    /* Return exit status */
    return exit_status;
}
