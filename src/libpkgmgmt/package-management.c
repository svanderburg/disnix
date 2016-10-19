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

#include "package-management.h"
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>

#define BUFFER_SIZE 1024
#define NIX_STORE_CMD "nix-store"
#define NIX_COLLECT_GARBAGE_CMD "nix-collect-garbage"
#define NIX_ENV_CMD "nix-env"

pid_t pkgmgmt_import_closure(int closure_fd, int stdout, int stderr)
{
    if(closure_fd == -1)
        return -1;
    else
    {
        pid_t pid = fork();
        
        if(pid == 0)
        {
            char *args[] = {NIX_STORE_CMD, "--import", NULL};
            
            dup2(closure_fd, 0);
            dup2(stdout, 1);
            dup2(stderr, 2);
            execvp(NIX_STORE_CMD, args);
            _exit(1);
        }
        
        return pid;
    }
}

gchar **pkgmgmt_export_closure(gchar *tmpdir, gchar **derivation, int stderr)
{
    gchar *tempfilename = g_strconcat(tmpdir, "/disnix.XXXXXX", NULL);
    int closure_fd = mkstemp(tempfilename);
    
    if(closure_fd == -1)
    {
        g_free(tempfilename);
        return NULL;
    }
    else
    {
        pid_t pid = fork();
        
        if(pid == -1)
        {
            close(closure_fd);
            g_free(tempfilename);
            return NULL;
        }
        else if(pid == 0)
        {
            unsigned int i, derivation_length = g_strv_length(derivation);
            gchar **args = (char**)g_malloc((3 + derivation_length) * sizeof(gchar*));

            args[0] = NIX_STORE_CMD;
            args[1] = "--export";

            for(i = 0; i < derivation_length; i++)
                args[i + 2] = derivation[i];

            args[i + 2] = NULL;

            dup2(closure_fd, 1);
            dup2(stderr, 2);
            execvp(NIX_STORE_CMD, args);
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
                    dprintf(stderr, "Cannot change permissions on exported closure: %s\n", tempfilename);
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

pid_t pkgmgmt_print_invalid_packages(gchar **derivation, int pipefd[2], int stderr)
{
    if(pipe(pipefd) == 0)
    {
        pid_t pid = fork();

        if(pid == 0)
        {
            unsigned int i, derivation_length = g_strv_length(derivation);
            gchar **args = (char**)g_malloc((4 + derivation_length) * sizeof(gchar*));

            close(pipefd[0]); /* Close read-end of the pipe */

            args[0] = NIX_STORE_CMD;
            args[1] = "--check-validity";
            args[2] = "--print-invalid";

            for(i = 0; i < derivation_length; i++)
                args[i + 3] = derivation[i];

            args[i + 3] = NULL;
            
            dup2(pipefd[1], 1); /* Attach write-end to stdout */
            dup2(stderr, 2); /* Attach logger to stderr */
            execvp(NIX_STORE_CMD, args);
            _exit(1);
        }
        
        return pid;
    }
    else
        return -1;
}

pid_t pkgmgmt_realise(gchar **derivation, int pipefd[2], int stderr)
{
    if(pipe(pipefd) == 0)
    {
        pid_t pid = fork();

        if(pid == 0)
        {
            unsigned int i, derivation_size = g_strv_length(derivation);
            gchar **args = (gchar**)g_malloc((3 + derivation_size) * sizeof(gchar*));

            close(pipefd[0]); /* Close read-end of pipe */

            args[0] = NIX_STORE_CMD;
            args[1] = "-r";

            for(i = 0; i < derivation_size; i++)
                args[i + 2] = derivation[i];

            args[i + 2] = NULL;

            dup2(pipefd[1], 1);
            dup2(stderr, 2);
            execvp(NIX_STORE_CMD, args);
            _exit(1);
        }
        
        return pid;
    }
    else
        return -1;
}

pid_t pkgmgmt_set_profile(gchar *profile, gchar *derivation, int stdout, int stderr)
{
    gchar *profile_path;
    ssize_t resolved_path_size;
    char resolved_path[BUFFER_SIZE];
    pid_t pid;
    
    mkdir(LOCALSTATEDIR "/nix/profiles/disnix", 0755);
    profile_path = g_strconcat(LOCALSTATEDIR "/nix/profiles/disnix/", profile, NULL);
    
    /* Resolve the manifest file to which the disnix profile points */
    resolved_path_size = readlink(profile_path, resolved_path, BUFFER_SIZE);
    
    if(resolved_path_size != -1 && (strlen(profile_path) != resolved_path_size || strncmp(resolved_path, profile_path, resolved_path_size) != 0)) /* If the symlink resolves not to itself, we get a generation symlink that we must resolve again */
    {
        gchar *generation_path;

        resolved_path[resolved_path_size] = '\0';
        
        generation_path = g_strconcat(LOCALSTATEDIR "/nix/profiles/disnix/", resolved_path, NULL);
        resolved_path_size = readlink(generation_path, resolved_path, BUFFER_SIZE);
        
        g_free(generation_path);
    }
    
    pid = fork();

    if(pid == 0)
    {
        if(resolved_path_size == -1 || (strlen(derivation) == resolved_path_size && strncmp(resolved_path, derivation, resolved_path_size) != 0)) /* Only configure the configurator profile if the given manifest is not identical to the previous manifest */
        {
            gchar *args[] = {NIX_ENV_CMD, "-p", profile_path, "--set", derivation, NULL};
            dup2(stdout, 1);
            dup2(stderr, 2);
            execvp(NIX_ENV_CMD, args);
            dprintf(stderr, "Error with executing nix-env\n");
            _exit(1);
        }
        else
            _exit(0);
    }
    
    g_free(profile_path);
    return pid;
}

pid_t pkgmgmt_query_requisites(gchar **derivation, int pipefd[2], int stderr)
{
    if(pipe(pipefd) == 0)
    {
        pid_t pid = fork();

        if(pid == 0)
        {
            unsigned int i, derivation_size = g_strv_length(derivation);
            char **args = (char**)g_malloc((3 + derivation_size) * sizeof(char*));

            close(pipefd[0]); /* Close read-end of pipe */

            args[0] = NIX_STORE_CMD;
            args[1] = "-qR";
            
            for(i = 0; i < derivation_size; i++)
                args[i + 2] = derivation[i];

            args[i + 2] = NULL;

            dup2(pipefd[1], 1);
            dup2(stderr, 2);
            execvp(NIX_STORE_CMD, args);
            _exit(1);
        }
        
        return pid;
    }
    else
        return -1;
}

pid_t pkgmgmt_collect_garbage(int delete_old, int stdout, int stderr)
{
    pid_t pid = fork();
    
    if(pid == 0)
    {
        dup2(stdout, 1);
        dup2(stderr, 2);
        
        if(delete_old)
        {
            char *args[] = {NIX_COLLECT_GARBAGE_CMD, "-d", NULL};
            execvp(NIX_COLLECT_GARBAGE_CMD, args);
        }
        else
        {
            char *args[] = {NIX_COLLECT_GARBAGE_CMD, NULL};
            execvp(NIX_COLLECT_GARBAGE_CMD, args);
        }
        
        dprintf(stderr, "Error with executing garbage collect process\n");
        _exit(1);
    }
    
    return pid;
}

pid_t pkgmgmt_instantiate(gchar *infrastructure_expr, int pipefd[2])
{
    if(pipe(pipefd) == 0)
    {
        pid_t pid = fork();
    
        if(pid == 0)
        {
            char *cmd = "nix-instantiate";
            char *const args[] = {cmd, "--eval-only", "--strict", "--xml", infrastructure_expr, NULL};
            close(pipefd[0]); /* Close read-end of pipe */
            dup2(pipefd[1], 1); /* Attach write-end to stdout */
            execvp(cmd, args);
            _exit(1);
        }
    
        return pid;
    }
    else
        return -1;
}

pid_t pkgmgmt_set_coordinator_profile(gchar *profile_path, gchar *manifest_file_path)
{
    pid_t pid = fork();
    
    if(pid == 0)
    {
        char *const args[] = {NIX_ENV_CMD, "-p", profile_path, "--set", manifest_file_path, NULL};
        execvp(NIX_ENV_CMD, args);
        _exit(1);
    }
    
    return pid;
}
