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

pid_t statemgmt_remote_activate(gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service)
{
    return exec_dysnomia_activity("--activate", interface, target, container, type, arguments, arguments_size, service);
}

pid_t statemgmt_remote_deactivate(gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service)
{
    return exec_dysnomia_activity("--deactivate", interface, target, container, type, arguments, arguments_size, service);
}

static pid_t lock_or_unlock(gchar *operation, gchar *interface, gchar *target, gchar *profile)
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

pid_t statemgmt_remote_lock(gchar *interface, gchar *target, gchar *profile)
{
    return lock_or_unlock("--lock", interface, target, profile);
}

pid_t statemgmt_remote_unlock(gchar *interface, gchar *target, gchar *profile)
{
    return lock_or_unlock("--unlock", interface, target, profile);
}

pid_t statemgmt_remote_snapshot(gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service)
{
    return exec_dysnomia_activity("--snapshot", interface, target, container, type, arguments, arguments_size, service);
}

pid_t statemgmt_remote_restore(gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service)
{
    return exec_dysnomia_activity("--restore", interface, target, container, type, arguments, arguments_size, service);
}

pid_t statemgmt_remote_delete_state(gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service)
{
    return exec_dysnomia_activity("--delete-state", interface, target, container, type, arguments, arguments_size, service);
}

pid_t statemgmt_remote_shell(gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service, gchar *command)
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

ProcReact_Future statemgmt_remote_capture_config(gchar *interface, gchar *target)
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

pid_t statemgmt_dummy_command(void)
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
