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

#include "collect-garbage.h"
#include <infrastructure.h>
#include <client-interface.h>

typedef struct
{
    const gboolean delete_old;
}
CollectGarbageData;

static pid_t collect_garbage_on_target(void *data, Target *target, gchar *client_interface, gchar *target_key)
{
    CollectGarbageData *collect_garbage_data = (CollectGarbageData*)data;
    g_print("[target: %s]: Running garbage collector\n", target_key);
    return exec_collect_garbage(client_interface, target_key, collect_garbage_data->delete_old);
}

static void complete_collect_garbage_on_target(void *data, Target *target, gchar *target_key, ProcReact_Status status, int result)
{
    if(status != PROCREACT_STATUS_OK || !result)
        g_printerr("[target: %s]: Garbage collection failed!\n", target_key);
}

int collect_garbage(gchar *interface, const gchar *target_property, gchar *infrastructure_expr, const gboolean delete_old)
{
    /* Retrieve an array of all target machines from the infrastructure expression */
    GPtrArray *target_array = create_target_array(infrastructure_expr);
    
    if(target_array == NULL)
    {
        g_printerr("[coordinator]: Error retrieving targets from infrastructure model!\n");
        return 1;
    }
    else
    {
        /* Iterate over all targets and run collect garbage operation in parallel */
        int success;
        CollectGarbageData data = { delete_old };
        ProcReact_PidIterator iterator = create_target_iterator(target_array, target_property, interface, collect_garbage_on_target, complete_collect_garbage_on_target, &data);
        
        procreact_fork_in_parallel_and_wait(&iterator);
        success = target_iterator_has_succeeded(&iterator);
        
        /* Cleanup */
        destroy_target_iterator(&iterator);
        delete_target_array(target_array);
        
        /* Return the exit status, which is 0 if everything succeeds */
        return (!success);
    }
}
