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

#include "state-management.h"
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>

#define BUFFER_SIZE 1024

pid_t statemgmt_run_dysnomia_activity(gchar *type, gchar *activity, gchar *component, gchar *container, gchar **arguments, int stdout, int stderr)
{
    pid_t pid = fork();
    
    if(pid == 0)
    {
        unsigned int i;
        char *cmd = "dysnomia";
        char *args[] = {cmd, "--type", type, "--operation", activity, "--component", component, "--container", container, "--environment", NULL};

        /* Compose environment variables out of the arguments */
        for(i = 0; i < g_strv_length(arguments); i++)
        {
            gchar **name_value_pair = g_strsplit(arguments[i], "=", 2);
            setenv(name_value_pair[0], name_value_pair[1], FALSE);
            g_strfreev(name_value_pair);
        }
        
        dup2(stdout, 1);
        dup2(stderr, 2);
        execvp(cmd, args);
        _exit(1);
    }
    
    return pid;
}

pid_t statemgmt_query_all_snapshots(gchar *container, gchar *component, int pipefd[2], int stderr)
{
    if(pipe(pipefd) == 0)
    {
        pid_t pid = fork();
        
        if(pid == 0)
        {
            char *args[] = {"dysnomia-snapshots", "--query-all", "--container", container, "--component", component, NULL};

            close(pipefd[0]); /* Close read-end of pipe */

            dup2(pipefd[1], 1);
            dup2(stderr, 2);
            execvp("dysnomia-snapshots", args);
            _exit(1);
        }
        
        return pid;
    }
    else
        return -1;
}

pid_t statemgmt_query_latest_snapshot(gchar *container, gchar *component, int pipefd[2], int stderr)
{
    if(pipe(pipefd) == 0)
    {
        pid_t pid = fork();
        
        if(pid == 0)
        {
            char *args[] = {"dysnomia-snapshots", "--query-latest", "--container", container, "--component", component, NULL};

            close(pipefd[0]); /* Close read-end of pipe */

            dup2(pipefd[1], 1);
            dup2(stderr, 2);
            execvp("dysnomia-snapshots", args);
            _exit(1);
        }
        
        return pid;
    }
    else
        return -1;
}

pid_t statemgmt_print_missing_snapshots(gchar **component, int pipefd[2], int stderr)
{
    if(pipe(pipefd) == 0)
    {
        pid_t pid = fork();
        
        if(pid == 0)
        {
            unsigned int i, component_size = g_strv_length(component);
            gchar **args = (gchar**)g_malloc((component_size + 3) * sizeof(gchar*));

            args[0] = "dysnomia-snapshots";
            args[1] = "--print-missing";

            for(i = 0; i < component_size; i++)
                args[i + 2] = component[i];
            
            args[i + 2] = NULL;

            close(pipefd[0]); /* Close read-end of pipe */

            dup2(pipefd[1], 1);
            dup2(stderr, 2);
            execvp("dysnomia-snapshots", args);
           _exit(1);
        }
        
        return pid;
    }
    else
        return -1;
}

pid_t statemgmt_import_snapshots(gchar *container, gchar *component, gchar **snapshots, int stdout, int stderr)
{
    pid_t pid = fork();
    
    if(pid == 0)
    {
        unsigned int i, snapshots_size = g_strv_length(snapshots);
        gchar **args = (gchar**)g_malloc((snapshots_size + 6) * sizeof(gchar*));
        
        args[0] = "dysnomia-snapshots";
        args[1] = "--import";
        args[2] = "--container";
        args[3] = container;
        args[4] = "--component";
        args[5] = component;
        
        for(i = 0; i < snapshots_size; i++)
            args[i + 6] = snapshots[i];
        
        args[i + 6] = NULL;
        
        dup2(stdout, 1);
        dup2(stderr, 2);
        execvp("dysnomia-snapshots", args);
        _exit(1);
    }
    
    return pid;
}

pid_t statemgmt_resolve_snapshots(gchar **snapshots, int pipefd[2], int stderr)
{
    if(pipe(pipefd) == 0)
    {
        pid_t pid = fork();
        
        if(pid == 0)
        {
            unsigned int i, snapshots_size = g_strv_length(snapshots);
            gchar **args = (gchar**)g_malloc((snapshots_size + 3) * sizeof(gchar*));

            args[0] = "dysnomia-snapshots";
            args[1] = "--resolve";
            
            for(i = 0; i < snapshots_size; i++)
                args[i + 2] = snapshots[i];

            args[i + 2] = NULL;
            close(pipefd[0]); /* Close read-end of pipe */

            dup2(pipefd[1], 1);
            dup2(stderr, 2);
            execvp("dysnomia-snapshots", args);
            _exit(1);
        }
    
        return pid;
    }
    else
        return -1;
}

pid_t statemgmt_clean_snapshots(gchar *keepStr, gchar *container, gchar *component, int stdout, int stderr)
{
    pid_t pid = fork();
    
    if(pid == 0)
    {
        char **args = (char**)g_malloc(9 * sizeof(gchar*));
        unsigned int count = 4;
        
        args[0] = "dysnomia-snapshots";
        args[1] = "--gc";
        args[2] = "--keep";
        args[3] = keepStr;
        
        if(g_strcmp0(container, "") != 0)
        {
            args[count] = "--container";
            count++;
            args[count] = container;
            count++;
        }
        
        if(g_strcmp0(component, "") != 0)
        {
            args[count] = "--component";
            count++;
            args[count] = component;
            count++;
        }
        
        args[count] = NULL;
        
        dup2(stdout, 1);
        dup2(stderr, 2);
        execvp("dysnomia-snapshots", args);
        dprintf(stderr, "Error with executing clean snapshots process\n");
        _exit(1);
    }
    
    return pid;
}

gchar **statemgmt_capture_config(gchar *tmpdir, int stderr)
{
    /* Declarations */
    gchar *tempfilename = g_strconcat(tmpdir, "/disnix.XXXXXX", NULL);
    int closure_fd;
    
    /* Execute command */
    closure_fd = mkstemp(tempfilename);
    
    if(closure_fd == -1)
    {
        dprintf(stderr, "Error opening tempfile!\n");
        g_free(tempfilename);
        return NULL;
    }
    else
    {
        int pid = fork();

        if(pid == -1)
        {
            dprintf(stderr, "Error with forking dysnomia-containers process!\n");
            g_free(tempfilename);
            close(closure_fd);
            return NULL;
        }
        else if(pid == 0)
        {
            char *args[] = { "dysnomia-containers", "--generate-expr", NULL };
            
            dup2(closure_fd, 1);
            dup2(stderr, 2);
            execvp("dysnomia-containers", args);
            _exit(1);
        }
        else
        {
            gchar **tempfilepaths;
            wait(&pid);

            if(WIFEXITED(pid) && WEXITSTATUS(pid) == 0)
            {
                tempfilepaths = (gchar**)g_malloc(2 * sizeof(gchar*));
                tempfilepaths[0] = tempfilename;
                tempfilepaths[1] = NULL;
                
                if(fchmod(closure_fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) == 1)
                    dprintf(stderr, "Cannot change permissions on captured expression: %s\n", tempfilename);
            }
            else
            {
                tempfilepaths = NULL;
                g_free(tempfilename);
            }
            
            close(closure_fd);
            return tempfilepaths;
        }
    }
}

static pid_t lock_or_unlock_component(gchar *operation, gchar *type, gchar *container, gchar *component, int stdout, int stderr)
{
    pid_t pid = fork();
    
    if(pid == 0)
    {
        char *cmd = "dysnomia";
        char *args[] = {cmd, "--type", type, "--operation", operation, "--container", container, "--component", component, "--environment", NULL};
        dup2(stdout, 1);
        dup2(stderr, 2);
        execvp(cmd, args);
        _exit(1);
    }
    
    return pid;
}

pid_t statemgmt_lock_component(gchar *type, gchar *container, gchar *component, int stdout, int stderr)
{
    return lock_or_unlock_component("lock", type, container, component, stdout, stderr);
}

pid_t statemgmt_unlock_component(gchar *type, gchar *container, gchar *component, int stdout, int stderr)
{
    return lock_or_unlock_component("unlock", type, container, component, stdout, stderr);
}
