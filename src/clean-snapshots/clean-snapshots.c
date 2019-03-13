/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2018  Sander van der Burg
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
#include <targets-iterator.h>
#include <client-interface.h>

typedef struct
{
    int keep;
    gchar *container;
    gchar *component;
}
CleanSnapshotsData;

static pid_t clean_snapshots_on_target(void *data, Target *target, gchar *client_interface, gchar *target_key)
{
    CleanSnapshotsData *clean_snapshots_data = (CleanSnapshotsData*)data;
    
    g_print("[target: %s]: Running snapshot garbage collector!\n", target_key);
    return exec_clean_snapshots(client_interface, target_key, clean_snapshots_data->keep, clean_snapshots_data->container, clean_snapshots_data->component);
}

static void complete_clean_snapshots_on_target(void *data, Target *target, gchar *target_key, ProcReact_Status status, int result)
{
    if(status != PROCREACT_STATUS_OK || !result)
        g_printerr("[target: %s]: Snapshot garbage collection failed\n", target_key);
}

int clean_snapshots(gchar *interface, const gchar *target_property, gchar *infrastructure_expr, int keep, gchar *container, gchar *component, const int xml)
{
    /* Retrieve an array of all target machines from the infrastructure expression */
    GPtrArray *target_array = create_target_array(infrastructure_expr, xml);

    if(target_array == NULL)
    {
        g_printerr("[coordinator]: Error retrieving targets from infrastructure model!\n");
        return 1;
    }
    else
    {
        /* Iterate over all targets and run clean snapshots operation in parallel */
        int success;
        CleanSnapshotsData data = { keep, container, component };
        ProcReact_PidIterator iterator = create_target_pid_iterator(target_array, target_property, interface, clean_snapshots_on_target, complete_clean_snapshots_on_target, &data);

        procreact_fork_in_parallel_and_wait(&iterator);
        success = target_iterator_has_succeeded(iterator.data);

        /* Cleanup */
        destroy_target_pid_iterator(&iterator);
        delete_target_array(target_array);

        /* Return the exit status, which is 0 if everything succeeds */
        return (!success);
    }
}
