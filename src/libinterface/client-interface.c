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

#include "client-interface.h"
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int wait_to_finish(const pid_t pid)
{
    if(pid == -1)
    {
	g_printerr("Error with forking process!\n");
	return -1;
    }
    else
    {
	int status;
	wait(&status);
	
	if(WIFEXITED(status))
	    return WEXITSTATUS(status);
	else
	    return 1;
    }
}

static pid_t exec_activate_or_deactivate(gchar *operation, gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service)
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
	    
        execvp(interface, args);
        _exit(1);
    }
    else
	return pid;
}

pid_t exec_activate(gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service)
{
    return exec_activate_or_deactivate("--activate", interface, target, container, type, arguments, arguments_size, service);
}

pid_t exec_deactivate(gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service)
{
    return exec_activate_or_deactivate("--deactivate", interface, target, container, type, arguments, arguments_size, service);
}

static pid_t exec_lock_or_unlock(gchar *operation, gchar *interface, gchar *target, gchar *profile)
{
    pid_t pid = fork();
    
    if(pid == 0)
    {
	char *const args[] = {interface, operation, "--target", target, "--profile", profile, NULL};
	execvp(interface, args);
	_exit(1);
    }
    else
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
    return exec_activate_or_deactivate("--snapshot", interface, target, container, type, arguments, arguments_size, service);
}

pid_t exec_restore(gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service)
{
    return exec_activate_or_deactivate("--restore", interface, target, container, type, arguments, arguments_size, service);
}

pid_t exec_delete_state(gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service)
{
    return exec_activate_or_deactivate("--delete-state", interface, target, container, type, arguments, arguments_size, service);
}

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
    else
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
    else
	return pid;
}

pid_t exec_query_installed(gchar *interface, gchar *target, gchar *profile)
{
    pid_t pid = fork();
    
    if(pid == 0)
    {
	char *const args[] = {interface, "--target", target, "--profile", profile, "--query-installed", NULL};
	execvp(interface, args);
	_exit(1);
    }
    else
	return pid;
}

static pid_t exec_copy_closure(gchar *operation, gchar *interface, gchar *target, gchar *component)
{
    pid_t pid = fork();
    
    if(pid == 0)
    {
	char *const args[] = {"disnix-copy-closure", operation, "--target", target, "--interface", interface, component, NULL};
	execvp("disnix-copy-closure", args);
	_exit(1);
    }
    else
	return pid;
}

pid_t exec_copy_closure_from(gchar *interface, gchar *target, gchar *component)
{
    return exec_copy_closure("--from", interface, target, component);
}

pid_t exec_copy_closure_to(gchar *interface, gchar *target, gchar *component)
{
    return exec_copy_closure("--to", interface, target, component);
}

static pid_t exec_copy_snapshots(gchar *operation, gchar *interface, gchar *target, gchar *container, gchar *component, gboolean all)
{
    pid_t pid = fork();
    
    if(pid == 0)
    {
	unsigned int args_length = 11;
	char **args;
	
	if(all)
	    args_length++;
	    
	args = (char**)g_malloc(args_length * sizeof(char*));
	args[0] = "disnix-copy-snapshots";
	args[1] = operation;
	args[2] = "--target";
	args[3] = target;
	args[4] = "--interface";
	args[5] = interface;
	args[6] = "--container";
	args[7] = container;
	args[8] = "--component";
	args[9] = component;
	
	if(all)
	{
	    args[10] = "--all";
	    args[11] = NULL;
	}
	else
	    args[10] = NULL;
	
	execvp("disnix-copy-snapshots", args);
	_exit(1);
    }
    else
	return pid;
}

pid_t exec_copy_snapshots_from(gchar *interface, gchar *target, gchar *container, gchar *component, gboolean all)
{
    return exec_copy_snapshots("--from", interface, target, container, component, all);
}

pid_t exec_copy_snapshots_to(gchar *interface, gchar *target, gchar *container, gchar *component, gboolean all)
{
    return exec_copy_snapshots("--to", interface, target, container, component, all);
}

pid_t exec_clean_snapshots(gchar *interface, gchar *target, int keep, char *container, char *component)
{
    char keepStr[15];
    pid_t pid = fork();
    
    sprintf(keepStr, "%d", keep);
    
    if(pid == 0)
    {
	char **args = (char**)g_malloc(11 * sizeof(char*));
	unsigned int count = 6;
	
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
    else
	return pid;
}

pid_t exec_realise(gchar *interface, gchar *target, gchar *derivation, int pipefd[2])
{
    if(pipe(pipefd) == 0)
    {
        pid_t pid = fork();
	
	if(pid == 0)
	{
	    char *const args[] = {interface, "--realise", "--target", target, derivation, NULL};
	    close(pipefd[0]); /* Close read-end */
	    dup2(pipefd[1], 1); /* Attach pipe to the stdout */
	    execvp(interface, args); /* Run process */
	    _exit(1);
	}
	else if(pid == -1)
	{
	    close(pipefd[0]); /* Close read-end */
	    close(pipefd[1]); /* Close write-end */
	    return pid;
	}
	else
	{
	    close(pipefd[1]); /* Close write-end */
	    return pid;
	}
    }
    else
	return -1;
}

pid_t exec_capture_config(gchar *interface, gchar *target, int pipefd[2])
{
    if(pipe(pipefd) == 0)
    {
        pid_t pid = fork();
	
	if(pid == 0)
	{
	    char *const args[] = {interface, "--capture-config", "--target", target, NULL};
	    close(pipefd[0]); /* Close read-end */
	    dup2(pipefd[1], 1); /* Attach pipe to the stdout */
	    execvp(interface, args); /* Run process */
	    _exit(1);
	}
	else if(pid == -1)
	{
	    close(pipefd[0]); /* Close read-end */
	    close(pipefd[1]); /* Close write-end */
	    return pid;
	}
	else
	{
	    close(pipefd[1]); /* Close write-end */
	    return pid;
	}
    }
    else
	return -1;
}

pid_t exec_true(void)
{
    pid_t pid = fork();
    
    if(pid == 0)
    {
	char *const args[] = {"true", NULL};
	execvp("true", args);
	_exit(1);
    }
    else
	return pid;
}
