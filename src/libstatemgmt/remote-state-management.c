/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2020  Sander van der Burg
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

#include "remote-state-management.h"
#include <sys/types.h>
#include <stdio.h>

static pid_t exec_dysnomia_activity(gchar *operation, gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service)
{
    pid_t pid = fork();

    if(pid == 0)
    {
        unsigned int i;
        char **args = (char**)g_malloc((10 + 2 * arguments_size) * sizeof(char*));

        args[0] = interface;
        args[1] = operation;
        args[2] = "--target";
        args[3] = target;
        args[4] = "--container";
        args[5] = container;
        args[6] = "--type";
        args[7] = type;

        for(i = 0; i < arguments_size * 2; i += 2)
        {
            args[i + 8] = "--arguments";
            args[i + 9] = arguments[i / 2];
        }

        args[i + 8] = service;
        args[i + 9] = NULL;

        /*
         * Attach process to its own process group to prevent them from being
         * interrupted by the shell session starting the process
         */
        setpgid(0, 0);

        execvp(interface, args);
        _exit(1);
    }

    return pid;
}

pid_t exec_activate(gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service)
{
    return exec_dysnomia_activity("--activate", interface, target, container, type, arguments, arguments_size, service);
}

pid_t exec_deactivate(gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service)
{
    return exec_dysnomia_activity("--deactivate", interface, target, container, type, arguments, arguments_size, service);
}

static pid_t exec_lock_or_unlock(gchar *operation, gchar *interface, gchar *target, gchar *profile)
{
    pid_t pid = fork();

    if(pid == 0)
    {
        char *const args[] = {interface, operation, "--target", target, "--profile", profile, NULL};

        /*
         * Attach process to its own process group to prevent them from being
         * interrupted by the shell session starting the process
         */
        setpgid(0, 0);

        execvp(interface, args);
        _exit(1);
    }

    return pid;
}

pid_t exec_lock(gchar *interface, gchar *target, gchar *profile)
{
    return exec_lock_or_unlock("--lock", interface, target, profile);
}

pid_t exec_unlock(gchar *interface, gchar *target, gchar *profile)
{
    return exec_lock_or_unlock("--unlock", interface, target, profile);
}

pid_t exec_snapshot(gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service)
{
    return exec_dysnomia_activity("--snapshot", interface, target, container, type, arguments, arguments_size, service);
}

pid_t exec_restore(gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service)
{
    return exec_dysnomia_activity("--restore", interface, target, container, type, arguments, arguments_size, service);
}

pid_t exec_delete_state(gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service)
{
    return exec_dysnomia_activity("--delete-state", interface, target, container, type, arguments, arguments_size, service);
}

pid_t exec_dysnomia_shell(gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service, gchar *command)
{
    pid_t pid = fork();

    if(pid == 0)
    {
        unsigned int i;
        char **args = (char**)g_malloc((12 + 2 * arguments_size) * sizeof(char*));

        args[0] = interface;
        args[1] = "--shell";
        args[2] = "--target";
        args[3] = target;
        args[4] = "--container";
        args[5] = container;
        args[6] = "--type";
        args[7] = type;

        for(i = 0; i < arguments_size * 2; i += 2)
        {
            args[i + 8] = "--arguments";
            args[i + 9] = arguments[i / 2];
        }

        args[i + 8] = service;

        if(command == NULL)
            args[i + 9] = NULL;
        else
        {
            args[i + 9] = "--command";
            args[i + 10] = command;
            args[i + 11] = NULL;
        }

        execvp(interface, args);
        _exit(1);
    }

    return pid;
}

pid_t exec_clean_snapshots(gchar *interface, gchar *target, int keep, char *container, char *component)
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

ProcReact_Future exec_capture_config(gchar *interface, gchar *target)
{
    ProcReact_Future future = procreact_initialize_future(procreact_create_string_array_type('\n'));

    if(future.pid == 0)
    {
        char *const args[] = {interface, "--capture-config", "--target", target, NULL};
        dup2(future.fd, 1); /* Attach pipe to the stdout */
        execvp(interface, args); /* Run process */
        _exit(1);
    }

    return future;
}

ProcReact_Future exec_query_all_snapshots(gchar *interface, gchar *target, gchar *container, gchar *component)
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

char **exec_query_all_snapshots_sync(gchar *interface, gchar *target, gchar *container, gchar *component)
{
    ProcReact_Status status;
    ProcReact_Future future = exec_query_all_snapshots(interface, target, container, component);
    char **result = procreact_future_get(&future, &status);

    if(status == PROCREACT_STATUS_OK)
        return result;
    else
        return NULL;
}

ProcReact_Future exec_query_latest_snapshot(gchar *interface, gchar *target, gchar *container, gchar *component)
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

char **exec_query_latest_snapshot_sync(gchar *interface, gchar *target, gchar *container, gchar *component)
{
    ProcReact_Status status;
    ProcReact_Future future = exec_query_latest_snapshot(interface, target, container, component);
    char **result = procreact_future_get(&future, &status);

    if(status == PROCREACT_STATUS_OK)
        return result;
    else
        return NULL;
}

ProcReact_Future exec_print_missing_snapshots(gchar *interface, gchar *target, gchar **snapshots, unsigned int snapshots_length)
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

char **exec_print_missing_snapshots_sync(gchar *interface, gchar *target, gchar **snapshots, unsigned int snapshots_length)
{
    ProcReact_Status status;
    ProcReact_Future future = exec_print_missing_snapshots(interface, target, snapshots, snapshots_length);
    char **result = procreact_future_get(&future, &status);

    if(status == PROCREACT_STATUS_OK)
        return result;
    else
        return NULL;
}

ProcReact_Future exec_resolve_snapshots(gchar *interface, gchar *target, gchar **snapshots, unsigned int snapshots_length)
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

char **exec_resolve_snapshots_sync(gchar *interface, gchar *target, gchar **snapshots, unsigned int snapshots_length)
{
    ProcReact_Status status;
    ProcReact_Future future = exec_resolve_snapshots(interface, target, snapshots, snapshots_length);
    char **result = procreact_future_get(&future, &status);

    if(status == PROCREACT_STATUS_OK)
        return result;
    else
        return NULL;
}

pid_t exec_import_local_snapshots(gchar *interface, gchar *target, gchar *container, gchar *component, gchar **snapshots, unsigned int snapshots_length)
{
    pid_t pid = fork();

    if(pid == 0)
    {
        unsigned int i;
        char **args = (char**)malloc((10 + snapshots_length) * sizeof(char*));
        args[0] = interface;
        args[1] = "--target";
        args[2] = target;
        args[3] = "--import-snapshots";
        args[4] = "--localfile";
        args[5] = "--container";
        args[6] = container;
        args[7] = "--component";
        args[8] = component;

        for(i = 0; i < snapshots_length; i++)
            args[i + 9] = snapshots[i];

        args[i + 9] = NULL;

        execvp(args[0], args);
        _exit(1);
    }

    return pid;
}

int exec_import_local_snapshots_sync(gchar *interface, gchar *target, gchar *container, gchar *component, gchar **snapshots, unsigned int snapshots_length)
{
    ProcReact_Status status;
    pid_t pid = exec_import_local_snapshots(interface, target, container, component, snapshots, snapshots_length);
    int exit_status = procreact_wait_for_boolean(pid, &status);
    return(status == PROCREACT_STATUS_OK && exit_status);
}

pid_t exec_import_remote_snapshots(gchar *interface, gchar *target, gchar *container, gchar *component, gchar **snapshots, unsigned int snapshots_length)
{
    pid_t pid = fork();

    if(pid == 0)
    {
        unsigned int i;
        char **args = (char**)malloc((10 + snapshots_length) * sizeof(char*));
        args[0] = interface;
        args[1] = "--target";
        args[2] = target;
        args[3] = "--import-snapshots";
        args[4] = "--remotefile";
        args[5] = "--container";
        args[6] = container;
        args[7] = "--component";
        args[8] = component;

        for(i = 0; i < snapshots_length; i++)
            args[i + 9] = snapshots[i];

        args[i + 9] = NULL;

        execvp(args[0], args);
        _exit(1);
    }

    return pid;
}

int exec_import_remote_snapshots_sync(gchar *interface, gchar *target, gchar *container, gchar *component, gchar **snapshots, unsigned int snapshots_length)
{
    ProcReact_Status status;
    pid_t pid = exec_import_remote_snapshots(interface, target, container, component, snapshots, snapshots_length);
    int exit_status = procreact_wait_for_boolean(pid, &status);
    return(status == PROCREACT_STATUS_OK && exit_status);
}

ProcReact_Future exec_export_remote_snapshots(gchar *interface, gchar *target, gchar **snapshots, unsigned int snapshots_length)
{
    ProcReact_Future future = procreact_initialize_future(procreact_create_string_array_type('\n'));

    if(future.pid == 0)
    {
        unsigned int i;
        char **args = (char**)malloc((5 + snapshots_length) * sizeof(char*));
        args[0] = interface;
        args[1] = "--target";
        args[2] = target;
        args[3] = "--export-snapshots";

        for(i = 0; i < snapshots_length; i++)
            args[i + 4] = snapshots[i];

        args[i + 4] = NULL;

        dup2(future.fd, 1);
        execvp(args[0], args);
        _exit(1);
    }

    return future;
}

char **exec_export_remote_snapshots_sync(gchar *interface, gchar *target, gchar **snapshots, unsigned int snapshots_length)
{
    ProcReact_Status status;
    ProcReact_Future future = exec_export_remote_snapshots(interface, target, snapshots, snapshots_length);
    char **result = procreact_future_get(&future, &status);

    if(status == PROCREACT_STATUS_OK)
        return result;
    else
        return NULL;
}

pid_t exec_true(void)
{
    pid_t pid = fork();

    if(pid == 0)
    {
        char *const args[] = {"true", NULL};
        execvp(args[0], args);
        _exit(1);
    }

    return pid;
}
