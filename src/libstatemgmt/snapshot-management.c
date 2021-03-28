/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2021  Sander van der Burg
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

#include "snapshot-management.h"
#include <stdio.h>

ProcReact_Future statemgmt_query_all_snapshots(gchar *container, gchar *component, int stderr_fd)
{
    ProcReact_Future future = procreact_initialize_future(procreact_create_string_array_type('\n'));

    if(future.pid == 0)
    {
        char *const args[] = {"dysnomia-snapshots", "--query-all", "--container", container, "--component", component, NULL};

        dup2(future.fd, 1);
        dup2(stderr_fd, 2);
        execvp(args[0], args);
        _exit(1);
    }

    return future;
}

char **statemgmt_query_all_snapshots_sync(gchar *container, gchar *component, int stderr_fd)
{
    ProcReact_Status status;
    ProcReact_Future future = statemgmt_query_all_snapshots(container, component, stderr_fd);
    char **result = procreact_future_get(&future, &status);

    if(status == PROCREACT_STATUS_OK)
        return result;
    else
        return NULL;
}

ProcReact_Future statemgmt_query_latest_snapshot(gchar *container, gchar *component, int stderr_fd)
{
    ProcReact_Future future = procreact_initialize_future(procreact_create_string_array_type('\n'));

    if(future.pid == 0)
    {
        char *const args[] = {"dysnomia-snapshots", "--query-latest", "--container", container, "--component", component, NULL};

        dup2(future.fd, 1);
        dup2(stderr_fd, 2);
        execvp(args[0], args);
        _exit(1);
    }

    return future;
}

char **statemgmt_query_latest_snapshot_sync(gchar *container, gchar *component, int stderr_fd)
{
    ProcReact_Status status;
    ProcReact_Future future = statemgmt_query_latest_snapshot(container, component, stderr_fd);
    char **result = procreact_future_get(&future, &status);

    if(status == PROCREACT_STATUS_OK)
        return result;
    else
        return NULL;
}

ProcReact_Future statemgmt_print_missing_snapshots(gchar **snapshots, const unsigned int snapshots_length, int stderr_fd)
{
    ProcReact_Future future = procreact_initialize_future(procreact_create_string_array_type('\n'));

    if(future.pid == 0)
    {
        unsigned int i;
        gchar **args = (gchar**)g_malloc((snapshots_length + 3) * sizeof(gchar*));

        args[0] = "dysnomia-snapshots";
        args[1] = "--print-missing";

        for(i = 0; i < snapshots_length; i++)
            args[i + 2] = snapshots[i];

        args[i + 2] = NULL;

        dup2(future.fd, 1);
        dup2(stderr_fd, 2);
        execvp(args[0], args);
       _exit(1);
    }

    return future;
}

char **statemgmt_print_missing_snapshots_sync(gchar **snapshots, const unsigned int snapshots_length, int stderr_fd)
{
    ProcReact_Status status;
    ProcReact_Future future = statemgmt_print_missing_snapshots(snapshots, snapshots_length, stderr_fd);
    char **result = procreact_future_get(&future, &status);

    if(status == PROCREACT_STATUS_OK)
        return result;
    else
        return NULL;
}

ProcReact_Future statemgmt_resolve_snapshots(gchar **snapshots, const unsigned int snapshots_length, int stderr_fd)
{
    ProcReact_Future future = procreact_initialize_future(procreact_create_string_array_type('\n'));

    if(future.pid == 0)
    {
        unsigned int i;
        gchar **args = (gchar**)g_malloc((snapshots_length + 3) * sizeof(gchar*));

        args[0] = "dysnomia-snapshots";
        args[1] = "--resolve";

        for(i = 0; i < snapshots_length; i++)
            args[i + 2] = snapshots[i];

        args[i + 2] = NULL;

        dup2(future.fd, 1);
        dup2(stderr_fd, 2);
        execvp(args[0], args);
        _exit(1);
    }

    return future;
}

char **statemgmt_resolve_snapshots_sync(gchar **snapshots, const unsigned int snapshots_length, int stderr_fd)
{
    ProcReact_Status status;
    ProcReact_Future future = statemgmt_resolve_snapshots(snapshots, snapshots_length, stderr_fd);
    char **result = procreact_future_get(&future, &status);

    if(status == PROCREACT_STATUS_OK)
        return result;
    else
        return NULL;
}

pid_t statemgmt_clean_snapshots(int keep, gchar *container, gchar *component, int stdout_fd, int stderr_fd)
{
    pid_t pid = fork();

    if(pid == 0)
    {
        char **args = (char**)g_malloc(9 * sizeof(gchar*));
        unsigned int count = 4;
        char keep_str[15];

        /* Convert keep value to string */
        sprintf(keep_str, "%d", keep);

        /* Compose command-line arguments */
        args[0] = "dysnomia-snapshots";
        args[1] = "--gc";
        args[2] = "--keep";
        args[3] = keep_str;

        if(g_strcmp0(container, "") != 0) /* Add container parameter, if requested */
        {
            args[count] = "--container";
            count++;
            args[count] = container;
            count++;
        }

        if(g_strcmp0(component, "") != 0) /* Add component parameter, if requested */
        {
            args[count] = "--component";
            count++;
            args[count] = component;
            count++;
        }

        args[count] = NULL;

        dup2(stdout_fd, 1);
        dup2(stderr_fd, 2);
        execvp(args[0], args);
        _exit(1);
    }

    return pid;
}

pid_t statemgmt_import_snapshots(gchar *container, gchar *component, gchar **resolved_snapshots, const unsigned int resolved_snapshots_length, int stdout_fd, int stderr_fd)
{
    pid_t pid = fork();

    if(pid == 0)
    {
        unsigned int i;
        gchar **args = (gchar**)g_malloc((resolved_snapshots_length + 6) * sizeof(gchar*));

        args[0] = "dysnomia-snapshots";
        args[1] = "--import";
        args[2] = "--container";
        args[3] = container;
        args[4] = "--component";
        args[5] = component;

        for(i = 0; i < resolved_snapshots_length; i++)
            args[i + 6] = resolved_snapshots[i];

        args[i + 6] = NULL;

        dup2(stdout_fd, 1);
        dup2(stderr_fd, 2);
        execvp(args[0], args);
        _exit(1);
    }

    return pid;
}

ProcReact_bool statemgmt_import_snapshots_sync(gchar *container, gchar *component, gchar **resolved_snapshots, const unsigned int resolved_snapshots_length, int stdout_fd, int stderr_fd)
{
    ProcReact_Status status;
    pid_t pid = statemgmt_import_snapshots(container, component, resolved_snapshots, resolved_snapshots_length, stdout_fd, stderr_fd);
    int exit_status = procreact_wait_for_boolean(pid, &status);
    return (status == PROCREACT_STATUS_OK && exit_status);
}
