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

static pid_t clean_snapshots_on_target(void *data, gchar *target_key, Target *target)
{
    CleanSnapshotsData *clean_snapshots_data = (CleanSnapshotsData*)data;

    g_print("[target: %s]: Running snapshot garbage collector!\n", target_key);
    return exec_clean_snapshots((char*)target->client_interface, target_key, clean_snapshots_data->keep, clean_snapshots_data->container, clean_snapshots_data->component);
}

static void complete_clean_snapshots_on_target(void *data, gchar *target_key, Target *target, ProcReact_Status status, int result)
{
    if(status != PROCREACT_STATUS_OK || !result)
        g_printerr("[target: %s]: Snapshot garbage collection failed\n", target_key);
}

int clean_snapshots(gchar *interface, gchar *target_property, gchar *infrastructure_expr, int keep, gchar *container, gchar *component, const int xml)
{
    /* Retrieve a table of all target machines from the infrastructure expression */
    GHashTable *targets_table = create_targets_table(infrastructure_expr, xml, target_property, interface);

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
            /* Iterate over all targets and run clean snapshots operation in parallel */
            CleanSnapshotsData data = { keep, container, component };
            ProcReact_PidIterator iterator = create_target_pid_iterator(targets_table, clean_snapshots_on_target, complete_clean_snapshots_on_target, &data);

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
