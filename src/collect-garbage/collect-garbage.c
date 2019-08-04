/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2019  Sander van der Burg
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
#include <targets-iterator.h>
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

int collect_garbage(gchar *interface, gchar *target_property, gchar *infrastructure_expr, const unsigned int flags)
{
    /* Retrieve an array of all target machines from the infrastructure expression */
    GHashTable *targets_table = create_targets_table(infrastructure_expr, flags & FLAG_COLLECT_GARBAGE_XML, target_property, interface);

    if(targets_table == NULL)
    {
        g_printerr("[coordinator]: Error retrieving targets from infrastructure model!\n");
        return 1;
    }
    else
    {
        int exit_status;

        if(check_targets_table(targets_table))
        {
            /* Iterate over all targets and run collect garbage operation in parallel */
            gboolean delete_old = flags & FLAG_COLLECT_GARBAGE_DELETE_OLD;
            CollectGarbageData data = { delete_old };
            ProcReact_PidIterator iterator = create_target_pid_iterator(targets_table, target_property, interface, collect_garbage_on_target, complete_collect_garbage_on_target, &data);

            procreact_fork_in_parallel_and_wait(&iterator);
            exit_status = !target_iterator_has_succeeded(iterator.data);

            /* Cleanup */
            destroy_target_pid_iterator(&iterator);
        }
        else
            exit_status = 1;

        delete_targets_table(targets_table);

        /* Return the exit status, which is 0 if everything succeeds */
        return exit_status;
    }
}
