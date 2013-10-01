/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2011  Sander van der Burg
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

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <glib.h>
#include "signals.h"
#define BUFFER_SIZE 1024

extern char *tmpdir;

extern struct sigact oldact;

static int job_counter = 0;

gchar **update_lines_vector(gchar **lines, char *buf)
{
    unsigned int i;
    unsigned int start_offset = 0;
    unsigned int lines_length; 
    
    /* If lines is NULL, first allocate memory */
    if(lines == NULL)
    {
	lines = (gchar**)g_malloc(sizeof(gchar*));
	lines[0] = NULL;
	lines_length = 0;
    }
    else
	lines_length = g_strv_length(lines);
    
    /* Check the buffer for lines */
    for(i = 0; i < strlen(buf); i++)
    {
	/* If a linefeed is found append the line to the lines vector */
	if(buf[i] == '\n')
	{
	    unsigned int line_size = i - start_offset + 1;
	    gchar *line = (gchar*)g_malloc((line_size + 1) * sizeof(gchar));
	    strncpy(line, buf + start_offset, line_size);
	    line[line_size] = '\0';
	    
	    start_offset = i + 1;
	    
	    if(lines_length == 0)
	    {
		lines_length++;
		lines = (gchar**)g_realloc(lines, (lines_length + 1) * sizeof(gchar*));
		lines[0] = line;
		lines[1] = NULL;
	    }
	    else
	    {
		gchar *last_line = lines[lines_length - 1];
		
		if(strlen(last_line) > 0 && last_line[strlen(last_line) - 1] == '\n')
		{
		    lines_length++;
		    lines = (gchar**)g_realloc(lines, (lines_length + 1) * sizeof(gchar*));
		    lines[lines_length - 1] = line;
		    lines[lines_length] = NULL;
		}
		else
		{
		    gchar *old_last_line = lines[lines_length - 1];
		    
		    lines[lines_length - 1] = g_strconcat(old_last_line, line, NULL);
		    
		    g_free(old_last_line);
		    g_free(line);
		}
	    }
	}
    }
    
    /* If there is trailing stuff, append it to the lines vector */
    if(start_offset < i)
    {
	unsigned int line_size = i - start_offset + 1;
	gchar *line = (gchar*)g_malloc((line_size + 1) * sizeof(gchar));
	strncpy(line, buf + start_offset, line_size);
	line[line_size] = '\0';

	lines_length++;
	lines = (gchar**)g_realloc(lines, (lines_length + 1) * sizeof(gchar*));
	lines[lines_length - 1] = line;
	lines[lines_length] = NULL;
    }
    
    /* Return modified lines vector */
    return lines;
}

static void print_derivations(gchar **derivation)
{
    unsigned int i;
    
    for(i = 0; i < g_strv_length(derivation); i++)
	g_print("%s ", derivation[i]);
}

/* Import method */

static void disnix_import_thread_func(DisnixObject *object, const gint pid, gchar *closure)
{
    /* Declarations */
    int closure_fd;
    
    /* Print log entry */
    g_print("Importing: %s\n", closure);
    
    /* Execute command */
    closure_fd = open(closure, O_RDONLY);
    
    if(closure_fd == -1)
    {
	g_printerr("Cannot open closure file!\n");
	disnix_emit_failure_signal(object, pid);
    }
    else
    {
	int status = fork();
    
	if(status == 0)
	{
	    char *args[] = {"nix-store", "--import", NULL};
	    dup2(closure_fd, 0);
	    execvp("nix-store", args);
	    _exit(1);
	}
    
	if(status == -1)
	{
	    g_printerr("Error with forking nix-store process!\n");
	    disnix_emit_failure_signal(object, pid);
	}
	else
	{
	    wait(&status);
	
	    if(WEXITSTATUS(status) == 0)
		disnix_emit_finish_signal(object, pid);
	    else
		disnix_emit_failure_signal(object, pid);
	}
    
	close(closure_fd);
    }
    
    _exit(0);
}

gboolean disnix_import(DisnixObject *object, const gint pid, gchar *closure, GError **error)
{
    /* State object should not be NULL */
    g_assert(object != NULL);
    
    /* Fork job process which returns a signal later */
    if(fork() == 0)
    {
	sigaction(SIGCHLD, (const struct sigaction *)&oldact, NULL);
	disnix_import_thread_func(object, pid, closure);
    }
    
    return TRUE;
}

/* Export method */

static void disnix_export_thread_func(DisnixObject *object, const gint pid, gchar **derivation)
{
    /* Declarations */
    char line[BUFFER_SIZE];
    char *tempfilename = g_strconcat(tmpdir, "/disnix.XXXXXX", NULL);
    int closure_fd;
    
    /* Print log entry */
    g_print("Exporting: ");
    print_derivations(derivation);
    g_print("\n");
    
    /* Execute command */
        
    closure_fd = mkstemp(tempfilename);
    
    if(closure_fd == -1)
    {
	g_printerr("Error opening tempfile!\n");
	disnix_emit_failure_signal(object, pid);
    }
    else
    {
	int status = fork();
    
	if(status == 0)
	{
	    unsigned int i, derivation_length = g_strv_length(derivation);
	    char **args = (char**)g_malloc((3 + derivation_length) * sizeof(char*));
	
	    args[0] = "nix-store";
	    args[1] = "--export";
	
	    for(i = 0; i < derivation_length; i++)
		args[i + 2] = derivation[i];
	
	    args[i + 2] = NULL;
	
	    dup2(closure_fd, 1);
	    execvp("nix-store", args);
	    _exit(1);
	}
	
	if(status == -1)
	{
	    g_printerr("Error with forking nix-store process!\n");
	    disnix_emit_failure_signal(object, pid);
	}
	else
	{
	    wait(&status);
	    
	    if(WEXITSTATUS(status) == 0)
	    {
		gchar *tempfilepaths[2];
		tempfilepaths[0] = tempfilename;
		tempfilepaths[1] = NULL;
		disnix_emit_success_signal(object, pid, tempfilepaths);
	    }
	    else
		disnix_emit_failure_signal(object, pid);
	}
	
	close(closure_fd);
    }
    
    /* Cleanup */
    g_free(tempfilename);
    
    _exit(0);
}

gboolean disnix_export(DisnixObject *object, const gint pid, gchar **derivation, GError **error)
{
    /* State object should not be NULL */
    g_assert(object != NULL);

    /* Fork job process which returns a signal later */
    if(fork() == 0)
    {
	sigaction(SIGCHLD, (const struct sigaction *)&oldact, NULL);
	disnix_export_thread_func(object, pid, derivation);
    }
        
    return TRUE;
}

/* Print invalid paths method */

static void disnix_print_invalid_thread_func(DisnixObject *object, const gint pid, gchar **derivation)
{
    /* Declarations */
    int pipefd[2];
    
    /* Print log entry */
    g_print("Print invalid: ");
    print_derivations(derivation);
    g_print("\n");
    
    /* Execute command */
    
    if(pipe(pipefd) == 0)
    {
	int status = fork();
	
	if(status == 0)
	{
	    unsigned int i, derivation_length = g_strv_length(derivation);
	    char **args = (char**)g_malloc((4 + derivation_length) * sizeof(char*));
	    
	    close(pipefd[0]); /* Close read-end of the pipe */
	
	    args[0] = "nix-store";
	    args[1] = "--check-validity";
	    args[2] = "--print-invalid";
	
	    for(i = 0; i < derivation_length; i++)	
		args[i + 3] = derivation[i];
	    
	    args[i + 3] = NULL;

	    dup2(pipefd[1], 1); /* Attach write-end to stdout */
	    execvp("nix-store", args);
	    _exit(1);
	}
    
	if(status == -1)
	{
	    g_printerr("Error with forking nix-store process!\n");
	    close(pipefd[0]);
	    close(pipefd[1]);
	    disnix_emit_failure_signal(object, pid);
	}
	else
	{
	    char line[BUFFER_SIZE];
	    ssize_t line_size;
	    gchar **missing_paths = NULL;
	    unsigned int missing_paths_size = 0;
	    
	    close(pipefd[1]); /* Close write-end of the pipe */
	    	
	    while((line_size = read(pipefd[0], line, BUFFER_SIZE - 1)) > 0)
	    {
	        line[line_size] = '\0';
	        g_print("%s", line);
				
		missing_paths = update_lines_vector(missing_paths, line);		
	    }
	
	    g_print("\n");
	    
	    close(pipefd[0]);
	    
	    wait(&status);
	
	    if(WEXITSTATUS(status) == 0)
		disnix_emit_success_signal(object, pid, missing_paths);
	    else
		disnix_emit_failure_signal(object, pid);
	    
	    g_strfreev(missing_paths);
	}
    }
    else
    {
	fprintf(stderr, "Error with creating a pipe!\n");
	disnix_emit_failure_signal(object, pid);
    }
        
    _exit(0);
}

gboolean disnix_print_invalid(DisnixObject *object, const gint pid, gchar **derivation, GError **error)
{
    /* State object should not be NULL */
    g_assert(object != NULL);

    /* Fork job process which returns a signal later */
    if(fork() == 0)
    {
	sigaction(SIGCHLD, (const struct sigaction *)&oldact, NULL);
	disnix_print_invalid_thread_func(object, pid, derivation);
    }
        
    return TRUE;
}

/* Realise method */

static void disnix_realise_thread_func(DisnixObject *object, const gint pid, gchar **derivation)
{
    /* Declarations */
    int pipefd[2];
    
    /* Print log entry */
    g_print("Realising: ");
    print_derivations(derivation);
    g_print("\n");
    
    /* Execute command */    

    if(pipe(pipefd) == 0)
    {
	int status = fork();
	
	if(status == 0)
	{
	    unsigned int i, derivation_size = g_strv_length(derivation);
	    char **args = (char**)g_malloc((3 + derivation_size) * sizeof(char*));
	    
	    close(pipefd[0]); /* Close read-end of pipe */
	    
	    args[0] = "nix-store";
	    args[1] = "-r";
	    
	    for(i = 0; i < derivation_size; i++)
		args[i + 2] = derivation[i];
	
	    args[i + 2] = NULL;
	    
	    dup2(pipefd[1], 1);
	    execvp("nix-store", args);
	    _exit(1);
	}
	
	if(status == -1)
	{
	    g_printerr("Error with forking nix-store process!\n");
	    disnix_emit_failure_signal(object, pid);
	}
	else
	{
	    char line[BUFFER_SIZE];
	    ssize_t line_size;
	    gchar **realised = NULL;
	    unsigned int realised_size = 0;
	
	    close(pipefd[1]); /* Close write-end of pipe */
	    
	    while((line_size = read(pipefd[0], line, BUFFER_SIZE - 1)) > 0)
	    {
	        line[line_size] = '\0';
		g_print("%s", line);
		realised = update_lines_vector(realised, line);
	    }

	    g_print("\n");
	    
	    close(pipefd[0]);
	    
	    wait(&status);
	    
	    if(WEXITSTATUS(status) == 0)
		disnix_emit_success_signal(object, pid, realised);
	    else
		disnix_emit_failure_signal(object, pid);
	    
	    g_strfreev(realised);
	}
    }
    else
    {
	g_printerr("Error with creating a pipe\n");
	disnix_emit_failure_signal(object, pid);
    }
    
    _exit(0);
}

gboolean disnix_realise(DisnixObject *object, const gint pid, gchar **derivation, GError **error)
{
    /* State object should not be NULL */
    g_assert(object != NULL);

    /* Fork job process which returns a signal later */
    if(fork() == 0)
    {
	sigaction(SIGCHLD, (const struct sigaction *)&oldact, NULL);
	disnix_realise_thread_func(object, pid, derivation);    
    }
    
    return TRUE;
}

/* Set method */

static void disnix_set_thread_func(DisnixObject *object, const gint pid, const gchar *profile, gchar *derivation)
{
    /* Declarations */
    gchar *profile_path;
    int status;
    char line[BUFFER_SIZE];
    
    /* Print log entry */
    g_print("Set profile: %s with derivation: %s\n", profile, derivation);
    
    /* Execute command */
    
    mkdir(LOCALSTATEDIR "/nix/profiles/disnix", 0755);
    profile_path = g_strconcat(LOCALSTATEDIR "/nix/profiles/disnix/", profile, NULL);

    status = fork();    
    
    if(status == -1)
    {
	g_printerr("Error with forking nix-env process!\n");
	disnix_emit_failure_signal(object, pid);
    }
    else if(status == 0)
    {
	char *args[] = {"nix-env", "-p", profile_path, "--set", derivation, NULL};
	execvp("nix-env", args);
	g_printerr("Error with executing nix-env\n");
	_exit(1);
    }
    
    if(status != -1)
    {
	wait(&status);
	
	if(WEXITSTATUS(status) == 0)
	    disnix_emit_finish_signal(object, pid);
	else
	    disnix_emit_failure_signal(object, pid);
    }
    
    /* Free variables */
    g_free(profile_path);
    
    _exit(0);
}

gboolean disnix_set(DisnixObject *object, const gint pid, const gchar *profile, gchar *derivation, GError **error)
{
    /* State object should not be NULL */
    g_assert(object != NULL);
    
    /* Fork job process which returns a signal later */
    if(fork() == 0)
    {
	sigaction(SIGCHLD, (const struct sigaction *)&oldact, NULL);
	disnix_set_thread_func(object, pid, profile, derivation);
    }
        
    return TRUE;
}

/* Query installed method */

static void disnix_query_installed_thread_func(DisnixObject *object, const gint pid, const gchar *profile)
{
    /* Declarations */
    gchar *cmd;
    FILE *fp;
    
    /* Print log entry */
    g_print("Query installed derivations from profile: %s\n", profile);
    
    /* Execute command */
    
    cmd = g_strconcat(LOCALSTATEDIR "/nix/profiles/disnix/", profile, "/manifest", NULL);

    fp = fopen(cmd, "r");
    if(fp == NULL)
	disnix_emit_failure_signal(object, pid); /* Something went wrong with forking the process */
    else
    {
	int status;
	char line[BUFFER_SIZE];
	gchar **derivation = NULL;
	unsigned int derivation_size = 0;
	int is_component = TRUE;
	
	/* Read the output */
	
	while(fgets(line, sizeof(line), fp) != NULL)
	{
	    puts(line);
	    
	    if(is_component)
	    {
		derivation = (gchar**)g_realloc(derivation, (derivation_size + 1) * sizeof(gchar*));
		derivation[derivation_size] = g_strdup(line);	    
		derivation_size++;
		is_component = FALSE;
	    }
	    else
		is_component = TRUE;
	}
	
	/* Add NULL value to the end of the list */
	derivation = (gchar**)g_realloc(derivation, (derivation_size + 1) * sizeof(gchar*));
	derivation[derivation_size] = NULL;
	
	fclose(fp);
	
	disnix_emit_success_signal(object, pid, derivation);
	
	g_strfreev(derivation);
    }
    
    _exit(0);
}

gboolean disnix_query_installed(DisnixObject *object, const gint pid, const gchar *profile, GError **error)
{
    /* State object should not be NULL */
    g_assert(object != NULL);
    
    /* Fork job process which returns a signal later */
    if(fork() == 0)
    {
	sigaction(SIGCHLD, (const struct sigaction *)&oldact, NULL);
	disnix_query_installed_thread_func(object, pid, profile);
    }
    
    return TRUE;
}

/* Query requisites method */

static void disnix_query_requisites_thread_func(DisnixObject *object, const gint pid, gchar **derivation)
{
    /* Declarations */
    int pipefd[2];
    
    /* Print log entry */
    g_print("Query requisites from derivations: ");
    print_derivations(derivation);
    g_print("\n");
    
    /* Execute command */
    
    if(pipe(pipefd) == 0)
    {
	int status = fork();
	
	if(status == 0)
	{
	    unsigned int i, derivation_size = g_strv_length(derivation);
	    char **args = (char**)g_malloc((3 + derivation_size) * sizeof(char*));
	    
	    close(pipefd[0]); /* Close read-end of pipe */
	    
	    args[0] = "nix-store";
	    args[1] = "-qR";
	    
	    for(i = 0; i < derivation_size; i++)
		args[i + 2] = derivation[i];
	
	    args[i + 2] = NULL;
	    
	    dup2(pipefd[1], 1);
	    execvp("nix-store", args);
	    _exit(1);
	}
	
	if(status == -1)
	{
	    fprintf(stderr, "Error with forking nix-store process!\n");
	    close(pipefd[0]);
	    close(pipefd[1]);
	    disnix_emit_failure_signal(object, pid);
	}
	else
	{
	    char line[BUFFER_SIZE];
	    ssize_t line_size;
	    gchar **requisites = NULL;
	    unsigned int requisites_size = 0;

	    close(pipefd[1]); /* Close write-end of pipe */
	    	
	    while((line_size = read(pipefd[0], line, BUFFER_SIZE - 1)) > 0)
	    {
	        line[line_size] = '\0';
		g_print("%s", line);
		requisites = update_lines_vector(requisites, line);
	    }
		
	    g_print("\n");
	    
	    close(pipefd[0]);

	    wait(&status);
	    
	    if(WEXITSTATUS(status) == 0)
		disnix_emit_success_signal(object, pid, requisites);
	    else
		disnix_emit_failure_signal(object, pid);
	
	    g_strfreev(requisites);
	}
    }
    else
    {
	g_printerr("Error with creating pipe!\n");
	disnix_emit_failure_signal(object, pid);
    }
        
    _exit(0);
}

gboolean disnix_query_requisites(DisnixObject *object, const gint pid, gchar **derivation, GError **error)
{
    /* State object should not be NULL */
    g_assert(object != NULL);
    
    /* Fork job process which returns a signal later */
    if(fork() == 0)
    {
	sigaction(SIGCHLD, (const struct sigaction *)&oldact, NULL);
	disnix_query_requisites_thread_func(object, pid, derivation);
    }
    
    return TRUE;
}

/* Garbage collect method */

static void disnix_collect_garbage_thread_func(DisnixObject *object, const gint pid, const gboolean delete_old)
{
    /* Declarations */
    int status;

    /* Print log entry */
    if(delete_old)
	g_print("Garbage collect and remove old derivations\n");
    else
	g_print("Garbage collect\n");
    
    /* Execute command */
    
    status = fork();
    
    if(status == -1)
    {
	g_printerr("Error with forking garbage collect process!\n");
	disnix_emit_failure_signal(object, pid);
    }
    else if(status == 0)
    {
	if(delete_old)
	{
	    char *args[] = {"nix-collect-garbage", NULL};
	    execvp("nix-collect-garbage", args);
	}
	else
	{
	    char *args[] = {"nix-collect-garbage", "-d", NULL};
	    execvp("nix-collect-garbage", args);
	}
	g_printerr("Error with executing garbage collect process\n");
	_exit(1);
    }
    
    if(status != -1)
    {
	wait(&status);
	
	if(WEXITSTATUS(status) == 0)
	    disnix_emit_finish_signal(object, pid);
	else
	    disnix_emit_failure_signal(object, pid);
    }
        
    _exit(0);
}

gboolean disnix_collect_garbage(DisnixObject *object, const gint pid, const gboolean delete_old, GError **error)
{
    /* State object should not be NULL */
    g_assert(object != NULL);

    /* Fork job process which returns a signal later */
    if(fork() == 0)
    {
	sigaction(SIGCHLD, (const struct sigaction *)&oldact, NULL);
	disnix_collect_garbage_thread_func(object, pid, delete_old);
    }
        
    return TRUE;
}

/* Activate method */

static void disnix_activate_thread_func(DisnixObject *object, const gint pid, gchar *derivation, gchar *type, gchar **arguments)
{
    /* Declarations */
    int status;
        
    /* Print log entry */
    g_print("Activate: %s of type: %s with arguments: ", derivation, type);
    print_derivations(arguments);
    g_print("\n");
    
    /* Execute command */

    status = fork();
    
    if(status == 0)
    {
	unsigned int i;
	char *cmd = "dysnomia";
	char *args[] = {cmd, "--type", type, "--operation", "activate", "--component", derivation, "--environment", NULL};
	
	for(i = 0; i < g_strv_length(arguments); i++)
	{
	    gchar **name_value_pair = g_strsplit(arguments[i], "=", 2);
	    setenv(name_value_pair[0], name_value_pair[1], FALSE);
	    g_strfreev(name_value_pair);
	}
	
	execvp(cmd, args);
	_exit(1);
    }
    
    if(status == -1)
    {
	g_printerr("Error forking activation process!\n");
	disnix_emit_failure_signal(object, pid);
    }
    else
    {
	wait(&status);
	
	if(WEXITSTATUS(status) == 0)
	    disnix_emit_finish_signal(object, pid);
	else
	    disnix_emit_failure_signal(object, pid);
    }
    
    _exit(0);
}

gboolean disnix_activate(DisnixObject *object, const gint pid, gchar *derivation, gchar *type, gchar **arguments, GError **error)
{
    /* State object should not be NULL */
    g_assert(object != NULL);

    /* Fork job process which returns a signal later */
    if(fork() == 0)
    {
	sigaction(SIGCHLD, (const struct sigaction *)&oldact, NULL);
	disnix_activate_thread_func(object, pid, derivation, type, arguments);
    }
    
    return TRUE;
}

/* Deactivate method */

static void disnix_deactivate_thread_func(DisnixObject *object, const gint pid, gchar *derivation, gchar *type, gchar **arguments)
{
    /* Declarations */
    int status;
        
    /* Print log entry */
    g_print("Deactivate: %s of type: %s with arguments: ", derivation, type);
    print_derivations(arguments);
    g_print("\n");
    
    /* Execute command */

    status = fork();
    
    if(status == 0)
    {
	unsigned int i;
	char *cmd = "dysnomia";
	char *args[] = {cmd, "--type", type, "--operation", "deactivate", "--component", derivation, "--environment", NULL};
	
	for(i = 0; i < g_strv_length(arguments); i++)
	{
	    gchar **name_value_pair = g_strsplit(arguments[i], "=", 2);
	    setenv(name_value_pair[0], name_value_pair[1], FALSE);
	    g_strfreev(name_value_pair);
	}
	
	execvp(cmd, args);
	_exit(1);
    }
    
    if(status == -1)
    {
	g_printerr("Error forking deactivation process!\n");
	disnix_emit_failure_signal(object, pid);
    }
    else
    {
	wait(&status);
	
	if(WEXITSTATUS(status) == 0)
	    disnix_emit_finish_signal(object, pid);
	else
	    disnix_emit_failure_signal(object, pid);
    }
    
    _exit(0);
}

gboolean disnix_deactivate(DisnixObject *object, const gint pid, gchar *derivation, gchar *type, gchar **arguments, GError **error)
{
    /* State object should not be NULL */
    g_assert(object != NULL);

    /* Fork job process which returns a signal later */
    if(fork() == 0)
    {
	sigaction(SIGCHLD, (const struct sigaction *)&oldact, NULL);
	disnix_deactivate_thread_func(object, pid, derivation, type, arguments);
    }
    
    return TRUE;
}

/* Lock method */

static int unlock_services(gchar **derivation, unsigned int derivation_size, gchar **type, unsigned int type_size)
{
    unsigned int i;
    int exit_status = TRUE;
    
    for(i = 0; i < derivation_size; i++)
    {
	int status; 
	    
	g_print("Notifying unlock on %s: of type: %s\n", derivation[i], type[i]);
	status = fork();
	    
	if(status == 0)
	{
	    char *cmd = "dysnomia";
	    char *args[] = {cmd, "--type", type[i], "--operation", "unlock", "--component", derivation[i], "--environment", NULL};
	    execvp(cmd, args);
	    _exit(1);
	}
	    
	if(status == -1)
	{
	    g_printerr("Error forking unlock process!\n");
	    exit_status = FALSE;
	}
	else
	{
	    wait(&status);
		
	    if(WEXITSTATUS(status) != 0)
	    {
	        g_printerr("Unlock failed!\n");
	        exit_status = FALSE;
	    }
	}
    }
    
    return exit_status;
}

static void disnix_lock_thread_func(DisnixObject *object, const gint pid, const gchar *profile)
{
    FILE *fp;
    gchar *cmd;
    gchar **derivation = NULL;
    unsigned int derivation_size = 0;
    gchar **type = NULL;
    unsigned int type_size = 0;
    
    /* Print log entry */
    g_print("Acquiring lock on profile: %s\n", profile);
    
    /* First check which services are currently active on this machine */
    cmd = g_strconcat(LOCALSTATEDIR "/nix/profiles/disnix/", profile, "/manifest", NULL);

    fp = fopen(cmd, "r");
    if(fp != NULL)
    {
	int status;
	char line[BUFFER_SIZE];
	int is_component = TRUE;
	
	/* Read the output */
	
	while(fgets(line, BUFFER_SIZE - 1, fp) != NULL)
	{
	    unsigned int line_length = strlen(line);
	    
	    /* Chop off the linefeed at the end */
	    if(line > 0)
	      line[line_length - 1] = '\0';
	      
	    puts(line);
	    
	    if(is_component)
	    {
		derivation = (gchar**)g_realloc(derivation, (derivation_size + 1) * sizeof(gchar*));
		derivation[derivation_size] = g_strdup(line);
		derivation_size++;
		is_component = FALSE;
	    }
	    else
	    {
		type = (gchar**)g_realloc(type, (type_size + 1) * sizeof(gchar*));
		type[type_size] = g_strdup(line);
		type_size++;
		is_component = TRUE;
	    }
	}
	
	/* Add NULL value to the end of the list */
	derivation = (gchar**)g_realloc(derivation, (derivation_size + 1) * sizeof(gchar*));
	derivation[derivation_size] = NULL;
	type = (gchar**)g_realloc(type, (type_size + 1) * sizeof(gchar*));
	type[type_size] = NULL;
	
	fclose(fp);
    }
    
    /* For every derivation we need a type */
    if(derivation_size == type_size)
    {
	unsigned int i;
	int exit_status = 0;
	
        /* Notify all currently running services that we want to acquire a lock */
        for(i = 0; i < derivation_size; i++)
        {
	    int status; 
	    
	    g_print("Notifying lock on %s: of type: %s\n", derivation[i], type[i]);
	    status = fork();
	    
	    if(status == 0)
	    {
		char *cmd = "dysnomia";
		char *args[] = {cmd, "--type", type[i], "--operation", "lock", "--component", derivation[i], "--environment", NULL};
		execvp(cmd, args);
		_exit(1);
	    }
	    
	    if(status == -1)
	    {
		g_printerr("Error forking lock process!\n");
		exit_status = -1;
	    }
	    else
	    {
		wait(&status);
		
		if(WEXITSTATUS(status) != 0)
		{
		    g_printerr("Lock rejected!\n");
		    exit_status = -1;
		}
	    }
        }
	
	/* If we did not receive any rejection, start the lock itself */
	if(exit_status == 0)
	{
	    int fd;
	    gchar *lock_filename = g_strconcat(tmpdir, "/disnix.lock", NULL);
	    
	    /* If no lock exists, try to create one */
	    if((fd = open(lock_filename, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
	    {
		unlock_services(derivation, derivation_size, type, type_size);
    		disnix_emit_failure_signal(object, pid); /* Lock already exists -> fail */
	    }
	    else
	    {
    		close(fd);

    		/* Send finish signal */
    		disnix_emit_finish_signal(object, pid);
	    }
	    
	    g_free(lock_filename);
	}
	else
	{
	    unlock_services(derivation, derivation_size, type, type_size);
	    disnix_emit_failure_signal(object, pid);
	}
    }
    else
    {
	g_printerr("Corrupt profile manifest: a service or type is missing!\n");
	disnix_emit_failure_signal(object, pid);
    }
    
    /* Cleanup */
    g_strfreev(derivation);
    g_strfreev(type);
    
    _exit(0);
}

gboolean disnix_lock(DisnixObject *object, const gint pid, const gchar *profile, GError **error)
{
    /* State object should not be NULL */
    g_assert(object != NULL);

    /* Fork job process which returns a signal later */
    if(fork() == 0)
    {
	sigaction(SIGCHLD, (const struct sigaction *)&oldact, NULL);
	disnix_lock_thread_func(object, pid, profile);
    }
    
    return TRUE;
}

/* Unlock method */

static void disnix_unlock_thread_func(DisnixObject *object, const gint pid, const gchar *profile)
{
    FILE *fp;
    gchar *cmd;
    gchar **derivation = NULL;
    unsigned int derivation_size = 0;
    gchar **type = NULL;
    unsigned int type_size = 0;
    int failed = FALSE;
    gchar *lock_filename = g_strconcat(tmpdir, "/disnix.lock", NULL);
    
    /* Print log entry */
    g_print("Releasing lock on profile: %s\n", profile);

    /* First check which services are currently active on this machine */
    cmd = g_strconcat(LOCALSTATEDIR "/nix/profiles/disnix/", profile, "/manifest", NULL);

    fp = fopen(cmd, "r");
    if(fp != NULL)
    {
	int status;
	char line[BUFFER_SIZE];
	int is_component = TRUE;
	
	/* Read the output */
	
	while(fgets(line, BUFFER_SIZE - 1, fp) != NULL)
	{
	    unsigned int line_length = strlen(line);
	    
	    /* Chop off the linefeed at the end */
	    if(line > 0)
	      line[line_length - 1] = '\0';

	    puts(line);
	    
	    if(is_component)
	    {
		derivation = (gchar**)g_realloc(derivation, (derivation_size + 1) * sizeof(gchar*));
		derivation[derivation_size] = g_strdup(line);	    
		derivation_size++;
		is_component = FALSE;
	    }
	    else
	    {
		type = (gchar**)g_realloc(type, (type_size + 1) * sizeof(gchar*));
		type[type_size] = g_strdup(line);	    
		type_size++;
		is_component = TRUE;
	    }
	}
	
	/* Add NULL value to the end of the list */
	derivation = (gchar**)g_realloc(derivation, (derivation_size + 1) * sizeof(gchar*));
	derivation[derivation_size] = NULL;
	type = (gchar**)g_realloc(type, (type_size + 1) * sizeof(gchar*));
	type[type_size] = NULL;	
	
	fclose(fp);
    }
    
    /* For every derivation we need a type */
    if(derivation_size == type_size)
    {
	if(!unlock_services(derivation, derivation_size, type, type_size))
	{
	    g_printerr("Failed to send unlock notification to old services!\n");
	    failed = TRUE;
	}
    }
    else
    {
	g_printerr("Corrupt profile manifest: a service or type is missing!\n");
	failed = TRUE;
    }

    if(unlink(lock_filename) == -1)
    {
	g_printerr("There is no lock file!\n");
	failed = TRUE; /* There was no lock -> fail */
    }
    
    if(failed)
	disnix_emit_failure_signal(object, pid); 
    else
	disnix_emit_finish_signal(object, pid);
    
    /* Cleanup */
    g_free(lock_filename);
    
    _exit(0);
}

gboolean disnix_unlock(DisnixObject *object, const gint pid, const gchar *profile, GError **error)
{
    /* State object should not be NULL */
    g_assert(object != NULL);

    /* Fork job process which returns a signal later */
    if(fork() == 0)
    {
	sigaction(SIGCHLD, (const struct sigaction *)&oldact, NULL);
	disnix_unlock_thread_func(object, pid, profile);
    }
    
    return TRUE;
}

/* Get job id method */

gboolean disnix_get_job_id(DisnixObject *object, gint *pid, GError **error)
{
    g_printerr("Assigned job id: %d\n", job_counter);
    *pid = job_counter;
    job_counter++;
    return TRUE;
}
