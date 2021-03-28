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

#include "remote-snapshot-management.h"
#include <stdio.h>

pid_t statemgmt_remote_clean_snapshots(gchar *interface, gchar *target, int keep, char *container, char *component)
{
    pid_t pid = fork();

    if(pid == 0)
    {
        char **args = (char**)g_malloc(11 * sizeof(char*));
        unsigned int count = 6;
        char keepStr[15];

        sprintf(keepStr, "%d", keep);

        args[0] = interface;
        args[1] = "--target";
        args[2] = target;
        args[3] = "--clean-snapshots";
        args[4] = "--keep";
        args[5] = keepStr;

        if(container != NULL)
        {
            args[count] = "--container";
            count++;
            args[count] = container;
            count++;
        }

        if(component != NULL)
        {
            args[count] = "--component";
            count++;
            args[count] = component;
            count++;
        }

        args[count] = NULL;

        execvp(interface, args);
        _exit(1);
    }

    return pid;
}

ProcReact_Future statemgmt_remote_query_all_snapshots(gchar *interface, gchar *target, gchar *container, gchar *component)
{
    ProcReact_Future future = procreact_initialize_future(procreact_create_string_array_type('\n'));

    if(future.pid == 0)
    {
        unsigned int count = 0;
        unsigned int num_of_extra_params = 0;

        if(container != NULL)
            num_of_extra_params += 2;

        if(component != NULL)
            num_of_extra_params += 2;

        char **args = (char**)malloc((5 + num_of_extra_params) * sizeof(char*));
        args[0] = interface;
        args[1] = "--target";
        args[2] = target;
        args[3] = "--query-all-snapshots";

        if(container != NULL)
        {
            args[count + 4] = "--container";
            args[count + 5] = container;
            count += 2;
        }

        if(component != NULL)
        {
            args[count + 4] = "--component";
            args[count + 5] = component;
            count += 2;
        }

        args[count + 4] = NULL;

        dup2(future.fd, 1);
        execvp(args[0], args);
        _exit(1);
    }

    return future;
}

char **statemgmt_remote_query_all_snapshots_sync(gchar *interface, gchar *target, gchar *container, gchar *component)
{
    ProcReact_Status status;
    ProcReact_Future future = statemgmt_remote_query_all_snapshots(interface, target, container, component);
    char **result = procreact_future_get(&future, &status);

    if(status == PROCREACT_STATUS_OK)
        return result;
    else
        return NULL;
}

ProcReact_Future statemgmt_remote_query_latest_snapshot(gchar *interface, gchar *target, gchar *container, gchar *component)
{
    ProcReact_Future future = procreact_initialize_future(procreact_create_string_array_type('\n'));

    if(future.pid == 0)
    {
        unsigned int count = 0;
        unsigned int num_of_extra_params = 0;

        if(container != NULL)
            num_of_extra_params += 2;

        if(component != NULL)
            num_of_extra_params += 2;

        char **args = (char**)malloc((5 + num_of_extra_params) * sizeof(char*));
        args[0] = interface;
        args[1] = "--target";
        args[2] = target;
        args[3] = "--query-latest-snapshot";

        if(container != NULL)
        {
            args[count + 4] = "--container";
            args[count + 5] = container;
            count += 2;
        }

        if(component != NULL)
        {
            args[count + 4] = "--component";
            args[count + 5] = component;
            count += 2;
        }

        args[count + 4] = NULL;

        dup2(future.fd, 1);
        execvp(args[0], args);
        _exit(1);
    }

    return future;
}

char **statemgmt_remote_query_latest_snapshot_sync(gchar *interface, gchar *target, gchar *container, gchar *component)
{
    ProcReact_Status status;
    ProcReact_Future future = statemgmt_remote_query_latest_snapshot(interface, target, container, component);
    char **result = procreact_future_get(&future, &status);

    if(status == PROCREACT_STATUS_OK)
        return result;
    else
        return NULL;
}

ProcReact_Future statemgmt_remote_print_missing_snapshots(gchar *interface, gchar *target, gchar **snapshots, const unsigned int snapshots_length)
{
    ProcReact_Future future = procreact_initialize_future(procreact_create_string_array_type('\n'));

    if(future.pid == 0)
    {
        unsigned int i;
        char **args = (char**)malloc((5 + snapshots_length) * sizeof(char*));
        args[0] = interface;
        args[1] = "--target";
        args[2] = target;
        args[3] = "--print-missing-snapshots";

        for(i = 0; i < snapshots_length; i++)
            args[i + 4] = snapshots[i];

        args[i + 4] = NULL;

        dup2(future.fd, 1);
        execvp(args[0], args);
        _exit(1);
    }

    return future;
}

char **statemgmt_remote_print_missing_snapshots_sync(gchar *interface, gchar *target, gchar **snapshots, const unsigned int snapshots_length)
{
    ProcReact_Status status;
    ProcReact_Future future = statemgmt_remote_print_missing_snapshots(interface, target, snapshots, snapshots_length);
    char **result = procreact_future_get(&future, &status);

    if(status == PROCREACT_STATUS_OK)
        return result;
    else
        return NULL;
}

ProcReact_Future statemgmt_remote_resolve_snapshots(gchar *interface, gchar *target, gchar **snapshots, const unsigned int snapshots_length)
{
    ProcReact_Future future = procreact_initialize_future(procreact_create_string_array_type('\n'));

    if(future.pid == 0)
    {
        unsigned int i;
        char **args = (char**)malloc((5 + snapshots_length) * sizeof(char*));
        args[0] = interface;
        args[1] = "--target";
        args[2] = target;
        args[3] = "--resolve-snapshots";

        for(i = 0; i < snapshots_length; i++)
            args[i + 4] = snapshots[i];

        args[i + 4] = NULL;

        dup2(future.fd, 1);
        execvp(args[0], args);
        _exit(1);
    }

    return future;
}

char **statemgmt_remote_resolve_snapshots_sync(gchar *interface, gchar *target, gchar **snapshots, const unsigned int snapshots_length)
{
    ProcReact_Status status;
    ProcReact_Future future = statemgmt_remote_resolve_snapshots(interface, target, snapshots, snapshots_length);
    char **result = procreact_future_get(&future, &status);

    if(status == PROCREACT_STATUS_OK)
        return result;
    else
        return NULL;
}

pid_t statemgmt_import_local_snapshots(gchar *interface, gchar *target, gchar *container, gchar *component, gchar **resolved_snapshots, const unsigned int resolved_snapshots_length)
{
    pid_t pid = fork();

    if(pid == 0)
    {
        unsigned int i;
        char **args = (char**)malloc((10 + resolved_snapshots_length) * sizeof(char*));
        args[0] = interface;
        args[1] = "--target";
        args[2] = target;
        args[3] = "--import-snapshots";
        args[4] = "--localfile";
        args[5] = "--container";
        args[6] = container;
        args[7] = "--component";
        args[8] = component;

        for(i = 0; i < resolved_snapshots_length; i++)
            args[i + 9] = resolved_snapshots[i];

        args[i + 9] = NULL;

        execvp(args[0], args);
        _exit(1);
    }

    return pid;
}

ProcReact_bool statemgmt_import_local_snapshots_sync(gchar *interface, gchar *target, gchar *container, gchar *component, gchar **resolved_snapshots, const unsigned int resolved_snapshots_length)
{
    ProcReact_Status status;
    pid_t pid = statemgmt_import_local_snapshots(interface, target, container, component, resolved_snapshots, resolved_snapshots_length);
    int exit_status = procreact_wait_for_boolean(pid, &status);
    return(status == PROCREACT_STATUS_OK && exit_status);
}

pid_t statemgmt_import_remote_snapshots(gchar *interface, gchar *target, gchar *container, gchar *component, gchar **resolved_snapshots, const unsigned int resolved_snapshots_length)
{
    pid_t pid = fork();

    if(pid == 0)
    {
        unsigned int i;
        char **args = (char**)malloc((10 + resolved_snapshots_length) * sizeof(char*));
        args[0] = interface;
        args[1] = "--target";
        args[2] = target;
        args[3] = "--import-snapshots";
        args[4] = "--remotefile";
        args[5] = "--container";
        args[6] = container;
        args[7] = "--component";
        args[8] = component;

        for(i = 0; i < resolved_snapshots_length; i++)
            args[i + 9] = resolved_snapshots[i];

        args[i + 9] = NULL;

        execvp(args[0], args);
        _exit(1);
    }

    return pid;
}

ProcReact_bool statemgmt_import_remote_snapshots_sync(gchar *interface, gchar *target, gchar *container, gchar *component, gchar **resolved_snapshots, const unsigned int resolved_snapshots_length)
{
    ProcReact_Status status;
    pid_t pid = statemgmt_import_remote_snapshots(interface, target, container, component, resolved_snapshots, resolved_snapshots_length);
    int exit_status = procreact_wait_for_boolean(pid, &status);
    return(status == PROCREACT_STATUS_OK && exit_status);
}

ProcReact_Future statemgmt_export_remote_snapshots(gchar *interface, gchar *target, gchar **resolved_snapshots, const unsigned int resolved_snapshots_length)
{
    ProcReact_Future future = procreact_initialize_future(procreact_create_string_array_type('\n'));

    if(future.pid == 0)
    {
        unsigned int i;
        char **args = (char**)malloc((5 + resolved_snapshots_length) * sizeof(char*));
        args[0] = interface;
        args[1] = "--target";
        args[2] = target;
        args[3] = "--export-snapshots";

        for(i = 0; i < resolved_snapshots_length; i++)
            args[i + 4] = resolved_snapshots[i];

        args[i + 4] = NULL;

        dup2(future.fd, 1);
        execvp(args[0], args);
        _exit(1);
    }

    return future;
}

char **statemgmt_export_remote_snapshots_sync(gchar *interface, gchar *target, gchar **resolved_snapshots, const unsigned int resolved_snapshots_length)
{
    ProcReact_Status status;
    ProcReact_Future future = statemgmt_export_remote_snapshots(interface, target, resolved_snapshots, resolved_snapshots_length);
    char **result = procreact_future_get(&future, &status);

    if(status == PROCREACT_STATUS_OK)
        return result;
    else
        return NULL;
}
