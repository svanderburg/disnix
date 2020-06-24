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

#include "remote-package-management.h"
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

pid_t exec_collect_garbage(gchar *interface, gchar *target, const gboolean delete_old)
{
    /* Declarations */
    pid_t pid;
    char *delete_old_arg;

    /* Determine whether to use the delete old option */
    if(delete_old)
        delete_old_arg = "-d";
    else
        delete_old_arg = NULL;

    /* Fork and execute the collect garbage process */
    pid = fork();

    if(pid == 0)
    {
        char *const args[] = {interface, "--target", target, "--collect-garbage", delete_old_arg, NULL};
        execvp(interface, args);
        _exit(1);
    }

    return pid;
}

pid_t exec_set(gchar *interface, gchar *target, gchar *profile, gchar *component)
{
    pid_t pid = fork();

    if(pid == 0)
    {
        char *const args[] = {interface, "--target", target, "--profile", profile, "--set", component, NULL};
        execvp(interface, args);
        _exit(1);
    }

    return pid;
}

ProcReact_Future exec_query_installed(gchar *interface, gchar *target, gchar *profile)
{
    ProcReact_Future future = procreact_initialize_future(procreact_create_string_type());

    if(future.pid == 0)
    {
        char *const args[] = {interface, "--target", target, "--profile", profile, "--query-installed", NULL};
        dup2(future.fd, 1); /* Attach pipe to the stdout */
        execvp(interface, args); /* Run process */
        _exit(1);
    }

    return future;
}

ProcReact_Future exec_realise(gchar *interface, gchar *target, gchar *derivation)
{
    ProcReact_Future future = procreact_initialize_future(procreact_create_string_array_type('\n'));

    if(future.pid == 0)
    {
        char *const args[] = {interface, "--realise", "--target", target, derivation, NULL};
        dup2(future.fd, 1); /* Attach pipe to the stdout */
        execvp(interface, args); /* Run process */
        _exit(1);
    }

    return future;
}

ProcReact_Future exec_query_requisites(gchar *interface, gchar *target, gchar **derivation, unsigned int derivation_length)
{
    ProcReact_Future future = procreact_initialize_future(procreact_create_string_array_type('\n'));

    if(future.pid == 0)
    {
        unsigned int i;
        char **args = (char**)malloc((5 + derivation_length) * sizeof(char*));
        args[0] = interface;
        args[1] = "--query-requisites";
        args[2] = "--target";
        args[3] = target;

        for(i = 0; i < derivation_length; i++)
            args[i + 4] = derivation[i];

        args[i + 4] = NULL;

        dup2(future.fd, 1); /* Attach pipe to the stdout */
        execvp(interface, args); /* Run process */
        _exit(1);
    }

    return future;
}

char **exec_query_requisites_sync(gchar *interface, gchar *target, gchar **derivation, unsigned int derivation_length)
{
    ProcReact_Status status;
    ProcReact_Future future = exec_query_requisites(interface, target, derivation, derivation_length);
    char **result = procreact_future_get(&future, &status);

    if(status == PROCREACT_STATUS_OK)
        return result;
    else
        return NULL;
}

ProcReact_Future exec_print_invalid(gchar *interface, gchar *target, gchar **paths, unsigned int paths_length)
{
    ProcReact_Future future = procreact_initialize_future(procreact_create_string_array_type('\n'));

    if(future.pid == 0)
    {
        unsigned int i;
        char **args = (char**)malloc((5 + paths_length) * sizeof(char*));
        args[0] = interface;
        args[1] = "--target";
        args[2] = target;
        args[3] = "--print-invalid";

        for(i = 0; i < paths_length; i++)
            args[i + 4] = paths[i];

        args[i + 4] = NULL;

        dup2(future.fd, 1); /* Attach pipe to the stdout */
        execvp(interface, args); /* Run process */
        _exit(1);
    }

    return future;
}

char **exec_print_invalid_sync(gchar *interface, gchar *target, gchar **paths, unsigned int paths_length)
{
    ProcReact_Future future = exec_print_invalid(interface, target, paths, paths_length);
    ProcReact_Status status;
    char **result = procreact_future_get(&future, &status);

    if(status == PROCREACT_STATUS_OK)
        return result;
    else
        return NULL;
}

pid_t exec_import_local_closure(gchar *interface, gchar *target, char *closure)
{
    pid_t pid = fork();

    if(pid == 0)
    {
        char *const args[] = {interface, "--import", "--target", target, "--localfile", closure, NULL};
        execvp(args[0], args);
        _exit(1);
    }

    return pid;
}

ProcReact_bool exec_import_local_closure_sync(gchar *interface, gchar *target, char *closure)
{
    ProcReact_Status status;
    pid_t pid = exec_import_local_closure(interface, target, closure);
    int exit_status = procreact_wait_for_boolean(pid, &status);
    return(status == PROCREACT_STATUS_OK && exit_status);
}

ProcReact_Future exec_export_remote_closure(gchar *interface, gchar *target, char **paths, unsigned int paths_length)
{
    ProcReact_Future future = procreact_initialize_future(procreact_create_string_type());

    if(future.pid == 0)
    {
        unsigned int i;
        char **args = (char**)malloc((paths_length + 6) * sizeof(char*));

        args[0] = interface;
        args[1] = "--target";
        args[2] = target;
        args[3] = "--export";
        args[4] = "--remotefile";

        for(i = 0; i < paths_length; i++)
            args[i + 5] = paths[i];

        args[i + 5] = NULL;

        dup2(future.fd, 1);
        execvp(args[0], args);
        _exit(1);
    }

    return future;
}

char *exec_export_remote_closure_sync(gchar *interface, gchar *target, char **paths, unsigned int paths_length)
{
    ProcReact_Status status;
    ProcReact_Future future = exec_export_remote_closure(interface, target, paths, paths_length);
    char *result = procreact_future_get(&future, &status);

    if(status == PROCREACT_STATUS_OK)
    {
        if(result[strlen(result) - 1] == '\n') // Strip trailing linefeed, if needed
            result[strlen(result) - 1] = '\0';

        return result;
    }
    else
        return NULL;
}
