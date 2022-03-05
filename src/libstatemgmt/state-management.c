/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2022  Sander van der Burg
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

#include "state-management.h"
#include <stdlib.h>
#include <stdio.h>

static pid_t run_dysnomia_activity(gchar *type, gchar *activity, gchar *component, gchar *container, char **arguments, int stdout_fd, int stderr_fd)
{
    pid_t pid = fork();

    if(pid == 0)
    {
        char *const args[] = {"dysnomia", "--type", type, "--operation", activity, "--component", component, "--container", container, "--environment", NULL};

        /* Add environment variables */
        unsigned int i = 0;

        while(arguments[i] != NULL)
        {
            putenv(arguments[i]);
            i++;
        }

        dup2(stdout_fd, 1);
        dup2(stderr_fd, 2);
        execvp(args[0], args);
        _exit(1);
    }

    return pid;
}

pid_t statemgmt_activate(gchar *type, gchar *component, gchar *container, char **arguments, int stdout_fd, int stderr_fd)
{
    return run_dysnomia_activity(type, "activate", component, container, arguments, stdout_fd, stderr_fd);
}

pid_t statemgmt_deactivate(gchar *type, gchar *component, gchar *container, char **arguments, int stdout_fd, int stderr_fd)
{
    return run_dysnomia_activity(type, "deactivate", component, container, arguments, stdout_fd, stderr_fd);
}

pid_t statemgmt_snapshot(gchar *type, gchar *component, gchar *container, char **arguments, int stdout_fd, int stderr_fd)
{
    return run_dysnomia_activity(type, "snapshot", component, container, arguments, stdout_fd, stderr_fd);
}

pid_t statemgmt_restore(gchar *type, gchar *component, gchar *container, char **arguments, int stdout_fd, int stderr_fd)
{
    return run_dysnomia_activity(type, "restore", component, container, arguments, stdout_fd, stderr_fd);
}

pid_t statemgmt_collect_garbage(gchar *type, gchar *component, gchar *container, char **arguments, int stdout_fd, int stderr_fd)
{
    return run_dysnomia_activity(type, "collect-garbage", component, container, arguments, stdout_fd, stderr_fd);
}

static pid_t lock_or_unlock(gchar *operation, gchar *type, gchar *container, gchar *component, int stdout_fd, int stderr_fd)
{
    pid_t pid = fork();

    if(pid == 0)
    {
        char *const args[] = {"dysnomia", "--type", type, "--operation", operation, "--container", container, "--component", component, "--environment", NULL};
        dup2(stdout_fd, 1);
        dup2(stderr_fd, 2);
        execvp(args[0], args);
        _exit(1);
    }

    return pid;
}

pid_t statemgmt_lock(gchar *type, gchar *container, gchar *component, int stdout_fd, int stderr_fd)
{
    return lock_or_unlock("lock", type, container, component, stdout_fd, stderr_fd);
}

pid_t statemgmt_unlock(gchar *type, gchar *container, gchar *component, int stdout_fd, int stderr_fd)
{
    return lock_or_unlock("unlock", type, container, component, stdout_fd, stderr_fd);
}

pid_t statemgmt_shell(gchar *type, gchar *component, gchar *container, char **arguments, gchar *command)
{
    pid_t pid = fork();

    if(pid == 0)
    {
        /* Add environment variables */
        unsigned int i = 0;

        while(arguments[i] != NULL)
        {
            putenv(arguments[i]);
            i++;
        }

        if(command == NULL)
        {
            char *const args[] = {"dysnomia", "--type", type, "--shell", "--component", component, "--container", container, "--environment", NULL};
            execvp(args[0], args);
        }
        else
        {
            char *const args[] = {"dysnomia", "--type", type, "--shell", "--component", component, "--container", container, "--environment", "--command", command, NULL};
            execvp(args[0], args);
        }

        _exit(1);
    }

    return pid;
}

gchar *statemgmt_capture_config(gchar *tmpdir, int stderr_fd, pid_t *pid, int *temp_fd)
{
    gchar *tempfilename = g_strconcat(tmpdir, "/disnix.XXXXXX", NULL);

    /* Compose a temp file */
    *temp_fd = mkstemp(tempfilename);

    if(*temp_fd == -1)
    {
        dprintf(stderr_fd, "Error opening tempfile!\n");
        g_free(tempfilename);
        return NULL;
    }
    else
    {
        /* Execute process capturing the config and writing it to a temp file */
        *pid = fork();

        if(*pid == 0)
        {
            char *const args[] = { "dysnomia-containers", "--generate-expr", NULL };

            dup2(*temp_fd, 1);
            dup2(stderr_fd, 2);
            execvp(args[0], args);
            _exit(1);
        }

        return tempfilename;
    }
}
