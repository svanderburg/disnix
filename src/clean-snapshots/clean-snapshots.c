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

#include "clean-snapshots.h"
#include <infrastructure.h>
#include <client-interface.h>
#include <procreact_pid_iterator.h>

typedef struct
{
    unsigned int index;
    unsigned int length;
    GPtrArray *target_array;
    const gchar *target_property;
    gchar *interface;
    int keep;
    gchar *container;
    gchar *component;
    int success;
}
TargetIteratorData;

static int has_next_target(void *data)
{
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)data;
    return target_iterator_data->index < target_iterator_data->length;
}

static pid_t next_clean_snapshot_process(void *data)
{
    pid_t pid;
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)data;
    
    Target *target = g_ptr_array_index(target_iterator_data->target_array, target_iterator_data->index);
    gchar *client_interface = target->client_interface;
    gchar *target_key = find_target_key(target, target_iterator_data->target_property);

    /* If no client interface is provided by the infrastructure model, use global one */
    if(client_interface == NULL)
        client_interface = target_iterator_data->interface;
    
    g_print("[target: %s]: Running snapshot garbage collector!\n", target_key);
    pid = exec_clean_snapshots(client_interface, target_key, target_iterator_data->keep, target_iterator_data->container, target_iterator_data->component);
    target_iterator_data->index++;
    
    return pid;
}

static void complete_clean_snapshot_process(void *data, pid_t pid, ProcReact_Status status, int result)
{
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)data;
    
    if(status != PROCREACT_STATUS_OK || !result)
        target_iterator_data->success = FALSE;
}

int clean_snapshots(gchar *interface, const gchar *target_property, gchar *infrastructure_expr, int keep, gchar *container, gchar *component)
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
        /* Iterate over all targets and run clean snapshots operation in parallel */
        TargetIteratorData data = { 0, target_array->len, target_array, target_property, interface, keep, container, component, TRUE };
        ProcReact_PidIterator iterator = procreact_initialize_pid_iterator(has_next_target, next_clean_snapshot_process, procreact_retrieve_boolean, complete_clean_snapshot_process, &data);
        
        procreact_fork_in_parallel_and_wait(&iterator);
        
        /* Delete the target array from memory */
        delete_target_array(target_array);
        
        /* Return the exit status, which is 0 if everything succeeds */
        return (!data.success);
    }
}
