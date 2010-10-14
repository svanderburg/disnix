/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2010  Sander van der Burg
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
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int wait_to_finish(pid_t pid)
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
	return WEXITSTATUS(status);
    }    
}

static pid_t exec_activate_or_deactivate(gchar *operation, gchar *interface, gchar *target, gchar *type, gchar **arguments, unsigned int arguments_size, gchar *service)
{
    pid_t pid = fork();
	
    if(pid == 0)
    {
	unsigned int i;
	char **args = (char**)g_malloc((8 + 2 * arguments_size) * sizeof(gchar*));
	    
	args[0] = interface;
	args[1] = operation;
	args[2] = "--target";
	args[3] = target;
	args[4] = "--type";
	args[5] = type;
	    
	for(i = 0; i < arguments_size * 2; i += 2)
	{
	    args[i + 6] = "--arguments";
    	    args[i + 7] = arguments[i / 2];
        }
	    
        args[i + 6] = service;
        args[i + 7] = NULL;
	    
        execvp(interface, args);
        _exit(1);	    
    }
    else
	return pid;
}

pid_t exec_activate(gchar *interface, gchar *target, gchar *type, gchar **arguments, unsigned int arguments_size, gchar *service)
{
    return exec_activate_or_deactivate("--activate", interface, target, type, arguments, arguments_size, service);
}

pid_t exec_deactivate(gchar *interface, gchar *target, gchar *type, gchar **arguments, unsigned int arguments_size, gchar *service)
{
    return exec_activate_or_deactivate("--deactivate", interface, target, type, arguments, arguments_size, service);
}

static pid_t exec_lock_or_unlock(gchar *operation, gchar *interface, gchar *target, gchar *profile)
{
    pid_t pid = fork();
    
    if(pid == 0)
    {
	char *args[] = {interface, operation, "--target", target, "--profile", profile, NULL};
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

pid_t exec_collect_garbage(gchar *interface, gchar *target, gboolean delete_old)
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
	char *args[] = {interface, "--target", target, "--collect-garbage", delete_old_arg, NULL};
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
        char *args[] = {interface, "--target", target, "--profile", profile, "--set", component, NULL};
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
	char *args[] = {interface, "--target", target, "--profile", profile, "--query-installed", NULL};
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
	char *args[] = {"disnix-copy-closure", operation, "--target", target, "--interface", interface, component, NULL};
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

pid_t exec_realise(gchar *interface, gchar *target, gchar *derivation, int pipefd[2])
{
    if(pipe(pipefd) == 0)
    {
        pid_t pid = fork();
	
	if(pid == 0)
	{
	    char *args[] = {interface, "--realise", "--target", target, derivation, NULL};
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
