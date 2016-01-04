/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2015  Sander van der Burg
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

#include "methods.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <glib.h>
#define BUFFER_SIZE 1024

extern char *tmpdir, *logdir;

extern int job_counter;

static gchar **update_lines_vector(gchar **lines, char *buf)
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
	lines_length = g_strv_length((gchar**)lines);
    
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
		gchar *const last_line = lines[lines_length - 1];
		
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

static gchar **allocate_empty_array_if_null(gchar **arr)
{
    if(arr == NULL)
    {
        arr = (gchar**)g_malloc(sizeof(gchar*));
        arr[0] = NULL;
    }
    
    return arr;
}

static void print_paths(int fd, gchar **derivation)
{
    unsigned int i;
    
    for(i = 0; i < g_strv_length(derivation); i++)
        dprintf(fd, "%s ", derivation[i]);
}

static int open_log_file(const gint pid)
{
    gchar pidStr[15];
    gchar *log_path;
    int log_fd;
    
    sprintf(pidStr, "%d", pid);
    
    mkdir(logdir, 0755);
    log_path = g_strconcat(logdir, "/", pidStr, NULL);
    log_fd = open(log_path, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    
    g_free(log_path);
    return log_fd;
}

/* Get job id method */

gboolean on_handle_get_job_id(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation)
{
    g_printerr("Assigned job id: %d\n", job_counter);
    org_nixos_disnix_disnix_complete_get_job_id(object, invocation, job_counter);
    job_counter++;
    return TRUE;
}

/* Import method */

typedef struct
{
    OrgNixosDisnixDisnix *object;
    gint arg_pid;
    gchar *arg_closure;
}
ImportParams;

static gpointer disnix_import_thread_func(gpointer data)
{
    ImportParams *params = (ImportParams*)data;
    int log_fd = open_log_file(params->arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
    }
    else
    {
        /* Declarations */
        int closure_fd;
        
        /* Print log entry */
        dprintf(log_fd, "Importing: %s\n", params->arg_closure);
    
        /* Execute command */
        closure_fd = open(params->arg_closure, O_RDONLY);
    
        if(closure_fd == -1)
        {
            dprintf(log_fd, "Cannot open closure file!\n");
            org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
        }
        else
        {
            int status = fork();

            if(status == -1)
            {
                dprintf(log_fd, "Error with forking nix-store process!\n");
                org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
            }
            else if(status == 0)
            {
                char *args[] = {"nix-store", "--import", NULL};
                dup2(closure_fd, 0);
                dup2(log_fd, 1);
                dup2(log_fd, 2);
                execvp("nix-store", args);
                _exit(1);
            }
            else
            {
                wait(&status);

                if(WEXITSTATUS(status) == 0)
                    org_nixos_disnix_disnix_emit_finish(params->object, params->arg_pid);
                else
                    org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
            }
    
            close(closure_fd);
        }
        
        close(log_fd);
    }
    
    /* Cleanup */
    g_free(params->arg_closure);
    g_free(params);
    
    return NULL;
}

gboolean on_handle_import(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_closure)
{
    GThread *thread;
    
    ImportParams *params = (ImportParams*)g_malloc(sizeof(ImportParams));
    params->object = object;
    params->arg_pid = arg_pid;
    params->arg_closure = g_strdup(arg_closure);

    thread = g_thread_new("import", disnix_import_thread_func, params);
    org_nixos_disnix_disnix_complete_import(object, invocation);
    g_thread_unref(thread);
    return TRUE;
}

/* Export method */

typedef struct
{
    OrgNixosDisnixDisnix *object;
    gint arg_pid;
    gchar **arg_derivation;
}
ExportParams;

static gpointer disnix_export_thread_func(gpointer data)
{
    ExportParams *params = (ExportParams*)data;
    int log_fd = open_log_file(params->arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
    }
    else
    {
        /* Declarations */
        gchar *tempfilename = g_strconcat(tmpdir, "/disnix.XXXXXX", NULL);
        int closure_fd;
        
        /* Print log entry */
        dprintf(log_fd, "Exporting: ");
        print_paths(log_fd, params->arg_derivation);
        dprintf(log_fd, "\n");
    
        /* Execute command */
    
        closure_fd = mkstemp(tempfilename);
    
        if(closure_fd == -1)
        {
            dprintf(log_fd, "Error opening tempfile!\n");
            org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
        }
        else
        {
            int status = fork();

            if(status == -1)
            {
                dprintf(log_fd, "Error with forking nix-store process!\n");
                org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
            }
            else if(status == 0)
            {
                unsigned int i, derivation_length = g_strv_length(params->arg_derivation);
                gchar **args = (char**)g_malloc((3 + derivation_length) * sizeof(gchar*));

                args[0] = "nix-store";
                args[1] = "--export";

                for(i = 0; i < derivation_length; i++)
                    args[i + 2] = params->arg_derivation[i];

                args[i + 2] = NULL;

                dup2(closure_fd, 1);
                dup2(log_fd, 2);
                execvp("nix-store", args);
                _exit(1);
            }
            else
            {
                wait(&status);

                if(WEXITSTATUS(status) == 0)
                {
                    const gchar *tempfilepaths[] = { tempfilename, NULL };
                    
                    if(fchmod(closure_fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) == 1)
                        dprintf(log_fd, "Cannot change permissions on exported closure: %s\n", tempfilename);
                    
                    org_nixos_disnix_disnix_emit_success(params->object, params->arg_pid, (const gchar**)tempfilepaths);
                }
                else
                    org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
            }

            close(closure_fd);
        }
        
        close(log_fd);
    
        /* Cleanup */
        g_free(tempfilename);
    }
    
    /* Cleanup */
    g_strfreev(params->arg_derivation);
    g_free(params);
    return NULL;
}


gboolean on_handle_export(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *const *arg_derivation)
{
    GThread *thread;
    
    ExportParams *params = (ExportParams*)g_malloc(sizeof(ExportParams));
    params->object = object;
    params->arg_pid = arg_pid;
    params->arg_derivation = g_strdupv((gchar**)arg_derivation);

    thread = g_thread_new("export", disnix_export_thread_func, params);
    org_nixos_disnix_disnix_complete_export(object, invocation);
    g_thread_unref(thread);
    return TRUE;
}

/* Print invalid paths method */

typedef struct
{
    OrgNixosDisnixDisnix *object;
    gint arg_pid;
    gchar **arg_derivation;
}
PrintInvalidParams;

static gpointer disnix_print_invalid_thread_func(gpointer data)
{
    PrintInvalidParams *params = (PrintInvalidParams*)data;
    int log_fd = open_log_file(params->arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile of pid: %d!\n", params->arg_pid);
        org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
    }
    else
    {
        /* Declarations */
        int pipefd[2];
        
        /* Print log entry */
        dprintf(log_fd, "Print invalid: ");
        print_paths(log_fd, params->arg_derivation);
        dprintf(log_fd, "\n");
        
        /* Execute command */
    
        if(pipe(pipefd) == 0)
        {
            int status = fork();

            if(status == -1)
            {
                dprintf(log_fd, "Error with forking nix-store process!\n");
                close(pipefd[0]);
                close(pipefd[1]);
                org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
            }
            else if(status == 0)
            {
                unsigned int i, derivation_length = g_strv_length(params->arg_derivation);
                gchar **args = (char**)g_malloc((4 + derivation_length) * sizeof(gchar*));

                close(pipefd[0]); /* Close read-end of the pipe */

                args[0] = "nix-store";
                args[1] = "--check-validity";
                args[2] = "--print-invalid";

                for(i = 0; i < derivation_length; i++)
                    args[i + 3] = params->arg_derivation[i];

                args[i + 3] = NULL;
                
                dup2(pipefd[1], 1); /* Attach write-end to stdout */
                dup2(log_fd, 2); /* Attach logger to stderr */
                execvp("nix-store", args);
                _exit(1);
            }
            else
            {
                char line[BUFFER_SIZE];
                ssize_t line_size;
                gchar **missing_paths = NULL;

                close(pipefd[1]); /* Close write-end of the pipe */

                while((line_size = read(pipefd[0], line, BUFFER_SIZE - 1)) > 0)
                {
                    line[line_size] = '\0';
                    missing_paths = update_lines_vector(missing_paths, line);
                }

                close(pipefd[0]);

                wait(&status);
                
                if(WEXITSTATUS(status) == 0)
                {
                    missing_paths = allocate_empty_array_if_null(missing_paths);
                    org_nixos_disnix_disnix_emit_success(params->object, params->arg_pid, (const gchar**)missing_paths);
                }
                else
                    org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);

                g_strfreev(missing_paths);
            }
        }
        else
        {
            dprintf(log_fd, "Error with creating a pipe!\n");
            org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
        }
        
        close(log_fd);
    }
    
    /* Cleanup */
    g_strfreev(params->arg_derivation);
    g_free(params);
    return NULL;
}

gboolean on_handle_print_invalid(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *const *arg_derivation)
{
    GThread *thread;
    
    PrintInvalidParams *params = (PrintInvalidParams*)g_malloc(sizeof(PrintInvalidParams));
    params->object = object;
    params->arg_pid = arg_pid;
    params->arg_derivation = g_strdupv((gchar**)arg_derivation);

    thread = g_thread_new("export", disnix_print_invalid_thread_func, params);
    org_nixos_disnix_disnix_complete_print_invalid(object, invocation);
    g_thread_unref(thread);
    return TRUE;
}

/* Realise method */

typedef struct
{
    OrgNixosDisnixDisnix *object;
    gint arg_pid;
    gchar **arg_derivation;
}
RealiseParams;

static gpointer disnix_realise_thread_func(gpointer data)
{
    RealiseParams *params = (RealiseParams*)data;
    int log_fd = open_log_file(params->arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
    }
    else
    {
        /* Declarations */
        int pipefd[2];
        
        /* Print log entry */
        dprintf(log_fd,"Realising: ");
        print_paths(log_fd, params->arg_derivation);
        dprintf(log_fd, "\n");
        
        /* Execute command */

        if(pipe(pipefd) == 0)
        {
            int status = fork();

            if(status == -1)
            {
                dprintf(log_fd, "Error with forking nix-store process!\n");
                org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
            }
            else if(status == 0)
            {
                unsigned int i, derivation_size = g_strv_length(params->arg_derivation);
                gchar **args = (gchar**)g_malloc((3 + derivation_size) * sizeof(gchar*));

                close(pipefd[0]); /* Close read-end of pipe */

                args[0] = "nix-store";
                args[1] = "-r";

                for(i = 0; i < derivation_size; i++)
                    args[i + 2] = params->arg_derivation[i];

                args[i + 2] = NULL;

                dup2(pipefd[1], 1);
                dup2(log_fd, 2);
                execvp("nix-store", args);
                _exit(1);
            }
            else
            {
                char line[BUFFER_SIZE];
                ssize_t line_size;
                gchar **realised = NULL;

                close(pipefd[1]); /* Close write-end of pipe */

                while((line_size = read(pipefd[0], line, BUFFER_SIZE - 1)) > 0)
                {
                    line[line_size] = '\0';
                    realised = update_lines_vector(realised, line);
                }

                close(pipefd[0]);

                wait(&status);

                if(WEXITSTATUS(status) == 0)
                {
                    realised = allocate_empty_array_if_null(realised);
                    org_nixos_disnix_disnix_emit_success(params->object, params->arg_pid, (const gchar**)realised);
                }
                else
                    org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);

                g_strfreev(realised);
            }
        }
        else
        {
            dprintf(log_fd, "Error with creating a pipe\n");
            org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
        }
        
        close(log_fd);
    }
    
    /* Cleanup */
    g_strfreev(params->arg_derivation);
    g_free(params);
    return NULL;
}

gboolean on_handle_realise(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *const *arg_derivation)
{
    GThread *thread;
    
    RealiseParams *params = (RealiseParams*)g_malloc(sizeof(RealiseParams));
    params->object = object;
    params->arg_pid = arg_pid;
    params->arg_derivation = g_strdupv((gchar**)arg_derivation);
    
    thread = g_thread_new("realise", disnix_realise_thread_func, params);
    org_nixos_disnix_disnix_complete_realise(object, invocation);
    g_thread_unref(thread);
    return TRUE;
}

/* Set method */

typedef struct
{
    OrgNixosDisnixDisnix *object;
    gint arg_pid;
    gchar *arg_profile;
    gchar *arg_derivation;
}
SetParams;

static gpointer disnix_set_thread_func(gpointer data)
{
    SetParams *params = (SetParams*)data;
    int log_fd = open_log_file(params->arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
    }
    else
    {
        /* Declarations */
        gchar *profile_path;
        int status;
        char resolved_path[BUFFER_SIZE];
        ssize_t resolved_path_size;
        
        /* Print log entry */
        dprintf(log_fd, "Set profile: %s with derivation: %s\n", params->arg_profile, params->arg_derivation);
    
        /* Execute command */
        mkdir(LOCALSTATEDIR "/nix/profiles/disnix", 0755);
        profile_path = g_strconcat(LOCALSTATEDIR "/nix/profiles/disnix/", params->arg_profile, NULL);
    
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

        if(resolved_path_size == -1 || (strlen(params->arg_derivation) == resolved_path_size && strncmp(resolved_path, params->arg_derivation, resolved_path_size) != 0)) /* Only configure the configurator profile if the given manifest is not identical to the previous manifest */
        {
            status = fork();
    
            if(status == -1)
            {
                dprintf(log_fd, "Error with forking nix-env process!\n");
                org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
            }
            else if(status == 0)
            {
                gchar *args[] = {"nix-env", "-p", profile_path, "--set", params->arg_derivation, NULL};
                dup2(log_fd, 1);
                dup2(log_fd, 2);
                execvp("nix-env", args);
                dprintf(log_fd, "Error with executing nix-env\n");
                _exit(1);
            }
            else
            {
                wait(&status);
                
                if(WEXITSTATUS(status) == 0)
                    org_nixos_disnix_disnix_emit_finish(params->object, params->arg_pid);
                else
                    org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
            }
        }
        else
            org_nixos_disnix_disnix_emit_finish(params->object, params->arg_pid);
    
        close(log_fd);
    
        /* Free variables */
        g_free(profile_path);
    }
    
    /* Cleanup */
    g_free(params->arg_profile);
    g_free(params->arg_derivation);
    g_free(params);
    return NULL;
}

gboolean on_handle_set(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_profile, const gchar *arg_derivation)
{
    GThread *thread;
    
    SetParams *params = (SetParams*)g_malloc(sizeof(SetParams));
    params->object = object;
    params->arg_pid = arg_pid;
    params->arg_profile = g_strdup(arg_profile);
    params->arg_derivation = g_strdup(arg_derivation);

    thread = g_thread_new("set", disnix_set_thread_func, params);
    org_nixos_disnix_disnix_complete_set(object, invocation);
    g_thread_unref(thread);
    return TRUE;
}

/* Query installed method */

typedef struct
{
    OrgNixosDisnixDisnix *object;
    gint arg_pid;
    gchar *arg_profile;
}
QueryInstalledParams;

static gpointer disnix_query_installed_thread_func(gpointer data)
{
    QueryInstalledParams *params = (QueryInstalledParams*)data;
    int log_fd = open_log_file(params->arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
    }
    else
    {
        /* Declarations */
        gchar *cmd;
        FILE *fp;
        
        /* Print log entry */
        dprintf(log_fd, "Query installed derivations from profile: %s\n", params->arg_profile);
    
        /* Execute command */
    
        cmd = g_strconcat(LOCALSTATEDIR "/nix/profiles/disnix/", params->arg_profile, "/manifest", NULL);

        fp = fopen(cmd, "r");
        if(fp == NULL)
            org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid); /* Something went wrong with forking the process */
        else
        {
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

            org_nixos_disnix_disnix_emit_success(params->object, params->arg_pid, (const gchar**)derivation);

            g_strfreev(derivation);
        }
        
        close(log_fd);
    }
    
    /* Cleanup */
    g_free(params->arg_profile);
    g_free(params);
    return NULL;
}

gboolean on_handle_query_installed(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_profile)
{
    GThread *thread;
    
    QueryInstalledParams *params = (QueryInstalledParams*)g_malloc(sizeof(QueryInstalledParams));
    params->object = object;
    params->arg_pid = arg_pid;
    params->arg_profile = g_strdup(arg_profile);
    
    thread = g_thread_new("query-installed", disnix_query_installed_thread_func, params);
    org_nixos_disnix_disnix_complete_query_installed(object, invocation);
    g_thread_unref(thread);
    return TRUE;
}

/* Query requisites method */

typedef struct
{
    OrgNixosDisnixDisnix *object;
    gint arg_pid;
    gchar **arg_derivation;
}
QueryRequisitesParams;

static gpointer disnix_query_requisites_thread_func(gpointer data)
{
    QueryRequisitesParams *params = (QueryRequisitesParams*)data;
    int log_fd = open_log_file(params->arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
    }
    else
    {
        /* Declarations */
        int pipefd[2];
        
        /* Print log entry */
        dprintf(log_fd, "Query requisites from derivations: ");
        print_paths(log_fd, params->arg_derivation);
        dprintf(log_fd, "\n");
    
        /* Execute command */
    
        if(pipe(pipefd) == 0)
        {
            int status = fork();

            if(status == -1)
            {
                dprintf(log_fd, "Error with forking nix-store process!\n");
                close(pipefd[0]);
                close(pipefd[1]);
                org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
            }
            else if(status == 0)
            {
                unsigned int i, derivation_size = g_strv_length(params->arg_derivation);
                char **args = (char**)g_malloc((3 + derivation_size) * sizeof(char*));

                close(pipefd[0]); /* Close read-end of pipe */

                args[0] = "nix-store";
                args[1] = "-qR";
                
                for(i = 0; i < derivation_size; i++)
                    args[i + 2] = params->arg_derivation[i];

                args[i + 2] = NULL;

                dup2(pipefd[1], 1);
                dup2(log_fd, 2);
                execvp("nix-store", args);
                _exit(1);
            }
            else
            {
                char line[BUFFER_SIZE];
                ssize_t line_size;
                gchar **requisites = NULL;

                close(pipefd[1]); /* Close write-end of pipe */

                while((line_size = read(pipefd[0], line, BUFFER_SIZE - 1)) > 0)
                {
                    line[line_size] = '\0';
                    requisites = update_lines_vector(requisites, line);
                }

                close(pipefd[0]);

                wait(&status);
                
                if(WEXITSTATUS(status) == 0)
                {
                    requisites = allocate_empty_array_if_null(requisites);
                    org_nixos_disnix_disnix_emit_success(params->object, params->arg_pid, (const gchar**)requisites);
                }
                else
                    org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);

               g_strfreev(requisites);
            }
        }
        else
        {
            dprintf(log_fd, "Error with creating pipe!\n");
            org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
        }
        
        close(log_fd);
    }
    
    /* Cleanup */
    g_strfreev(params->arg_derivation);
    g_free(params);
    return NULL;
}

gboolean on_handle_query_requisites(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *const *arg_derivation)
{
    GThread *thread;
    
    QueryRequisitesParams *params = (QueryRequisitesParams*)g_malloc(sizeof(QueryRequisitesParams));
    params->object = object;
    params->arg_pid = arg_pid;
    params->arg_derivation = g_strdupv((gchar**)arg_derivation);
    
    thread = g_thread_new("query-requisites", disnix_query_requisites_thread_func, params);
    org_nixos_disnix_disnix_complete_query_requisites(object, invocation);
    g_thread_unref(thread);
    return TRUE;
}

/* Garbage collect method */

typedef struct
{
    OrgNixosDisnixDisnix *object;
    gint arg_pid;
    gboolean arg_delete_old;
}
CollectGarbageParams;

static gpointer disnix_collect_garbage_thread_func(gpointer data)
{
    CollectGarbageParams *params = (CollectGarbageParams*)data;
    int log_fd = open_log_file(params->arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
    }
    else
    {
        /* Declarations */
        int status;
        
        /* Print log entry */
        if(params->arg_delete_old)
            dprintf(log_fd, "Garbage collect and remove old derivations\n");
        else
            dprintf(log_fd, "Garbage collect\n");
    
        /* Execute command */
        status = fork();
    
        if(status == -1)
        {
            dprintf(log_fd, "Error with forking garbage collect process!\n");
            org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
        }
        else if(status == 0)
        {
            dup2(log_fd, 1);
            dup2(log_fd, 2);
            
            if(params->arg_delete_old)
            {
                char *args[] = {"nix-collect-garbage", "-d", NULL};
                execvp("nix-collect-garbage", args);
            }
            else
            {
                char *args[] = {"nix-collect-garbage", NULL};
                execvp("nix-collect-garbage", args);
            }
            dprintf(log_fd, "Error with executing garbage collect process\n");
            _exit(1);
        }
        else
        {
            wait(&status);

            if(WEXITSTATUS(status) == 0)
                org_nixos_disnix_disnix_emit_finish(params->object, params->arg_pid);
            else
                org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
        }
        
        close(log_fd);
    }
    
    /* Cleanup */
    g_free(params);
    return NULL;
}

gboolean on_handle_collect_garbage(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, gboolean arg_delete_old)
{
    GThread *thread;
    
    CollectGarbageParams *params = (CollectGarbageParams*)g_malloc(sizeof(CollectGarbageParams));
    params->object = object;
    params->arg_pid = arg_pid;
    params->arg_delete_old = arg_delete_old;
    
    thread = g_thread_new("collect_garbage", disnix_collect_garbage_thread_func, params);
    org_nixos_disnix_disnix_complete_collect_garbage(object, invocation);
    g_thread_unref(thread);
    return TRUE;
}

/* Common dysnomia invocation function */

typedef struct
{
    OrgNixosDisnixDisnix *object;
    gchar *activity;
    gint arg_pid;
    gchar *arg_derivation;
    gchar *arg_type;
    gchar **arg_arguments;
}
ActivityParams;

static ActivityParams *create_activity_params(OrgNixosDisnixDisnix *object, gchar *activity, gint arg_pid, const gchar *arg_derivation, const gchar *arg_type, const gchar *const *arg_arguments)
{
    ActivityParams *params = (ActivityParams*)g_malloc(sizeof(ActivityParams));
    
    params->object = object;
    params->activity = g_strdup(activity);
    params->arg_pid = arg_pid;
    params->arg_derivation = g_strdup(arg_derivation);
    params->arg_type = g_strdup(arg_type);
    params->arg_arguments = g_strdupv((gchar**)arg_arguments);
    
    return params;
}

static gpointer disnix_dysnomia_activity_thread_func(gpointer data)
{
    ActivityParams *params = (ActivityParams*)data;
    int log_fd = open_log_file(params->arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
    }
    else
    {
        /* Declarations */
        int status;
        
        /* Print log entry */
        dprintf(log_fd, "%s: %s of type: %s with arguments: ", params->activity, params->arg_derivation, params->arg_type);
        print_paths(log_fd, params->arg_arguments);
        dprintf(log_fd, "\n");
    
        /* Execute command */
        
        status = fork();
    
        if(status == -1)
        {
            dprintf(log_fd, "Error forking %s process!\n", params->activity);
            org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
        }
        else if(status == 0)
        {
            unsigned int i;
            char *cmd = "dysnomia";
            char *args[] = {cmd, "--type", params->arg_type, "--operation", params->activity, "--component", params->arg_derivation, "--environment", NULL};

            /* Compose environment variables out of the arguments */
            for(i = 0; i < g_strv_length(params->arg_arguments); i++)
            {
                gchar **name_value_pair = g_strsplit(params->arg_arguments[i], "=", 2);
                setenv(name_value_pair[0], name_value_pair[1], FALSE);
                g_strfreev(name_value_pair);
            }
            
            dup2(log_fd, 1);
            dup2(log_fd, 2);
            execvp(cmd, args);
            _exit(1);
        }
        else
        {
            wait(&status);

            if(WEXITSTATUS(status) == 0)
                org_nixos_disnix_disnix_emit_finish(params->object, params->arg_pid);
            else
                org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
        }
    
        close(log_fd);
    }
    
    /* Cleanup */
    g_free(params->activity);
    g_free(params->arg_derivation);
    g_free(params->arg_type);
    g_strfreev(params->arg_arguments);
    g_free(params);
    return NULL;
}

/* Activate method */

gboolean on_handle_activate(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_derivation, const gchar *arg_type, const gchar *const *arg_arguments)
{
    ActivityParams *params = create_activity_params(object, "activate", arg_pid, arg_derivation, arg_type, arg_arguments);
    GThread *thread = g_thread_new("activate", disnix_dysnomia_activity_thread_func, params);
    org_nixos_disnix_disnix_complete_activate(object, invocation);
    g_thread_unref(thread);
    return TRUE;
}

/* Deactivate method */

gboolean on_handle_deactivate(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_derivation, const gchar *arg_type, const gchar *const *arg_arguments)
{
    ActivityParams *params = create_activity_params(object, "deactivate", arg_pid, arg_derivation, arg_type, arg_arguments);
    GThread *thread = g_thread_new("deactivate", disnix_dysnomia_activity_thread_func, params);
    org_nixos_disnix_disnix_complete_deactivate(object, invocation);
    g_thread_unref(thread);
    return TRUE;
}

/* Lock method */

static int unlock_services(int log_fd, gchar **derivation, unsigned int derivation_size, gchar **type, unsigned int type_size)
{
    unsigned int i;
    int exit_status = TRUE;
    
    for(i = 0; i < derivation_size; i++)
    {
	int status; 
	
	dprintf(log_fd, "Notifying unlock on %s: of type: %s\n", derivation[i], type[i]);
	status = fork();
	
	if(status == -1)
	{
	    dprintf(log_fd, "Error forking unlock process!\n");
	    exit_status = FALSE;
	}
	else if(status == 0)
	{
	    char *cmd = "dysnomia";
	    char *args[] = {cmd, "--type", type[i], "--operation", "unlock", "--component", derivation[i], "--environment", NULL};
	    execvp(cmd, args);
	    _exit(1);
	}
	else
	{
	    wait(&status);
	
	    if(WEXITSTATUS(status) != 0)
	    {
	        dprintf(log_fd, "Unlock failed!\n");
	        exit_status = FALSE;
	    }
	}
    }
    
    return exit_status;
}

typedef struct
{
    OrgNixosDisnixDisnix *object;
    gint arg_pid;
    gchar *arg_profile;
}
LockParams;

static gpointer disnix_lock_thread_func(gpointer data)
{
    LockParams *params = (LockParams*)data;
    int log_fd = open_log_file(params->arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
    }
    else
    {
        FILE *fp;
        gchar *cmd;
        gchar **derivation = NULL;
        unsigned int derivation_size = 0;
        gchar **type = NULL;
        unsigned int type_size = 0;
        
        /* Print log entry */
        dprintf(log_fd, "Acquiring lock on profile: %s\n", params->arg_profile);
        
        /* First check which services are currently active on this machine */
        cmd = g_strconcat(LOCALSTATEDIR "/nix/profiles/disnix/", params->arg_profile, "/manifest", NULL);
    
        fp = fopen(cmd, "r");
        if(fp != NULL)
        {
            char line[BUFFER_SIZE];
            int is_component = TRUE;

            /* Read the output */

            while(fgets(line, BUFFER_SIZE - 1, fp) != NULL)
            {
                unsigned int line_length = strlen(line);
                
                /* Chop off the linefeed at the end */
                if(line > 0)
                    line[line_length - 1] = '\0';
                
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
    
                dprintf(log_fd, "Notifying lock on %s: of type: %s\n", derivation[i], type[i]);
                status = fork();

                if(status == -1)
                {
                    dprintf(log_fd, "Error forking lock process!\n");
                    exit_status = -1;
                }
                else if(status == 0)
                {
                    char *cmd = "dysnomia";
                    char *args[] = {cmd, "--type", type[i], "--operation", "lock", "--component", derivation[i], "--environment", NULL};
                    dup2(log_fd, 1);
                    dup2(log_fd, 2);
                    execvp(cmd, args);
                    _exit(1);
                }
                else
                {
                    wait(&status);

                    if(WEXITSTATUS(status) != 0)
                    {
                        dprintf(log_fd, "Lock rejected!\n");
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
                    unlock_services(log_fd, derivation, derivation_size, type, type_size);
                    org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid); /* Lock already exists -> fail */
                }
                else
                {
                    close(fd);

                    /* Send finish signal */
                    org_nixos_disnix_disnix_emit_finish(params->object, params->arg_pid);
                }

                g_free(lock_filename);
            }
            else
            {
                unlock_services(log_fd, derivation, derivation_size, type, type_size);
                org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
            }
        }
        else
        {
            dprintf(log_fd, "Corrupt profile manifest: a service or type is missing!\n");
            org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
        }
    
        close(log_fd);
    
        /* Cleanup */
        g_strfreev(derivation);
        g_strfreev(type);
    }
    
    /* Cleanup */
    g_free(params->arg_profile);
    g_free(params);
    return NULL;
}

gboolean on_handle_lock(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_profile)
{
    GThread *thread;
    
    LockParams *params = (LockParams*)g_malloc(sizeof(LockParams));
    params->object = object;
    params->arg_pid = arg_pid;
    params->arg_profile = g_strdup(arg_profile);
    
    thread = g_thread_new("lock", disnix_lock_thread_func, params);
    org_nixos_disnix_disnix_complete_lock(object, invocation);
    g_thread_unref(thread);
    return TRUE;
}

/* Unlock method */

typedef struct
{
    OrgNixosDisnixDisnix *object;
    gint arg_pid;
    gchar *arg_profile;
}
UnlockParams;

static gpointer disnix_unlock_thread_func(gpointer data)
{
    UnlockParams *params = (UnlockParams*)data;
    int log_fd = open_log_file(params->arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
    }
    else
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
        dprintf(log_fd, "Releasing lock on profile: %s\n", params->arg_profile);

        /* First check which services are currently active on this machine */
        cmd = g_strconcat(LOCALSTATEDIR "/nix/profiles/disnix/", params->arg_profile, "/manifest", NULL);

        fp = fopen(cmd, "r");
        if(fp != NULL)
        {
            char line[BUFFER_SIZE];
            int is_component = TRUE;
            
            /* Read the output */

            while(fgets(line, BUFFER_SIZE - 1, fp) != NULL)
            {
                unsigned int line_length = strlen(line);

                /* Chop off the linefeed at the end */
                if(line > 0)
                    line[line_length - 1] = '\0';

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
            if(!unlock_services(log_fd, derivation, derivation_size, type, type_size))
            {
                dprintf(log_fd, "Failed to send unlock notification to old services!\n");
                failed = TRUE;
            }
        }
        else
        {
            dprintf(log_fd, "Corrupt profile manifest: a service or type is missing!\n");
            failed = TRUE;
        }

        if(unlink(lock_filename) == -1)
        {
            dprintf(log_fd, "There is no lock file!\n");
            failed = TRUE; /* There was no lock -> fail */
        }
        
        if(failed)
            org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
        else
            org_nixos_disnix_disnix_emit_finish(params->object, params->arg_pid);
    
        close(log_fd);
    
        /* Cleanup */
        g_free(lock_filename);
    }
    
    /* Cleanup */
    g_free(params->arg_profile);
    g_free(params);
    return NULL;
}

gboolean on_handle_unlock(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_profile)
{
    GThread *thread;
    
    UnlockParams *params = (UnlockParams*)g_malloc(sizeof(UnlockParams));
    params->object = object;
    params->arg_pid = arg_pid;
    params->arg_profile = g_strdup(arg_profile);

    thread = g_thread_new("unlock", disnix_unlock_thread_func, params);
    org_nixos_disnix_disnix_complete_unlock(object, invocation);
    g_thread_unref(thread);
    return TRUE;
}

/* Snapshot method */

gboolean on_handle_snapshot(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_derivation, const gchar *arg_type, const gchar *const *arg_arguments)
{
    ActivityParams *params = create_activity_params(object, "snapshot", arg_pid, arg_derivation, arg_type, arg_arguments);
    GThread *thread = g_thread_new("snapshot", disnix_dysnomia_activity_thread_func, params);
    org_nixos_disnix_disnix_complete_snapshot(object, invocation);
    g_thread_unref(thread);
    return TRUE;
}

/* Restore method */

gboolean on_handle_restore(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_derivation, const gchar *arg_type, const gchar *const *arg_arguments)
{
    ActivityParams *params = create_activity_params(object, "restore", arg_pid, arg_derivation, arg_type, arg_arguments);
    GThread *thread = g_thread_new("restore", disnix_dysnomia_activity_thread_func, params);
    org_nixos_disnix_disnix_complete_restore(object, invocation);
    g_thread_unref(thread);
    return TRUE;
}

/* Query all snapshots method */

typedef struct
{
    OrgNixosDisnixDisnix *object;
    gint arg_pid;
    gchar *arg_container;
    gchar *arg_component;
}
QueryAllSnapshotsParams;

static gpointer disnix_query_all_snapshots_thread_func(gpointer data)
{
    QueryAllSnapshotsParams *params = (QueryAllSnapshotsParams*)data;
    int log_fd = open_log_file(params->arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
    }
    else
    {
        /* Declarations */
        int pipefd[2];
        
        /* Print log entry */
        dprintf(log_fd, "Query all snapshots from container: %s and component: %s\n", params->arg_container, params->arg_component);
    
        /* Execute command */
        
        if(pipe(pipefd) == 0)
        {
            int status = fork();

            if(status == -1)
            {
                dprintf(log_fd, "Error with forking dysnomia-snapshots process!\n");
                close(pipefd[0]);
                close(pipefd[1]);
                org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
            }
            else if(status == 0)
            {
                char *args[] = {"dysnomia-snapshots", "--query-all", "--container", params->arg_container, "--component", params->arg_component, NULL};

                close(pipefd[0]); /* Close read-end of pipe */

                dup2(pipefd[1], 1);
                dup2(log_fd, 2);
                execvp("dysnomia-snapshots", args);
                _exit(1);
            }
            else
            {
                char line[BUFFER_SIZE];
                ssize_t line_size;
                gchar **snapshots = NULL;

                close(pipefd[1]); /* Close write-end of pipe */

                while((line_size = read(pipefd[0], line, BUFFER_SIZE - 1)) > 0)
                {
                    line[line_size] = '\0';
                    snapshots = update_lines_vector(snapshots, line);
                }

                close(pipefd[0]);

                wait(&status);

                if(WEXITSTATUS(status) == 0)
                {
                    snapshots = allocate_empty_array_if_null(snapshots);
                    org_nixos_disnix_disnix_emit_success(params->object, params->arg_pid, (const gchar **)snapshots);
                }
                else
                    org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);

               g_strfreev(snapshots);
            }
        }
        else
        {
            dprintf(log_fd, "Error with creating pipe!\n");
            org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
        }
    
        close(log_fd);
    }
    
    /* Cleanup */
    g_free(params->arg_container);
    g_free(params->arg_component);
    g_free(params);
    return NULL;
}

gboolean on_handle_query_all_snapshots(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_container, const gchar *arg_component)
{
    GThread *thread;
    
    QueryAllSnapshotsParams *params = (QueryAllSnapshotsParams*)g_malloc(sizeof(QueryAllSnapshotsParams));
    params->object = object;
    params->arg_pid = arg_pid;
    params->arg_container = g_strdup(arg_container);
    params->arg_component = g_strdup(arg_component);
    
    thread = g_thread_new("query-all-snapshots", disnix_query_all_snapshots_thread_func, params);
    org_nixos_disnix_disnix_complete_query_all_snapshots(object, invocation);
    g_thread_unref(thread);
    return TRUE;
}

/* Query latest snapshot method */

typedef struct
{
    OrgNixosDisnixDisnix *object;
    gint arg_pid;
    gchar *arg_container;
    gchar *arg_component;
}
QueryLatestSnapshotParams;

static gpointer disnix_query_latest_snapshot_thread_func(gpointer data)
{
    QueryLatestSnapshotParams *params = (QueryLatestSnapshotParams*)data;
    int log_fd = open_log_file(params->arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
    }
    else
    {
        /* Declarations */
        int pipefd[2];
        
        /* Print log entry */
        dprintf(log_fd, "Query latest snapshot from container: %s and component: %s\n", params->arg_container, params->arg_component);
    
        /* Execute command */
    
        if(pipe(pipefd) == 0)
        {
            int status = fork();

            if(status == -1)
            {
                dprintf(log_fd, "Error with forking dysnomia-snapshots process!\n");
                close(pipefd[0]);
                close(pipefd[1]);
                org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
            }
            else if(status == 0)
            {
                char *args[] = {"dysnomia-snapshots", "--query-latest", "--container", params->arg_container, "--component", params->arg_component, NULL};

                close(pipefd[0]); /* Close read-end of pipe */

                dup2(pipefd[1], 1);
                dup2(log_fd, 2);
                execvp("dysnomia-snapshots", args);
                _exit(1);
            }
            else
            {
                char line[BUFFER_SIZE];
                ssize_t line_size;
                gchar **snapshots = NULL;

                close(pipefd[1]); /* Close write-end of pipe */

                while((line_size = read(pipefd[0], line, BUFFER_SIZE - 1)) > 0)
                {
                    line[line_size] = '\0';
                    snapshots = update_lines_vector(snapshots, line);
                }

                close(pipefd[0]);

                wait(&status);

                if(WEXITSTATUS(status) == 0)
                {
                    snapshots = allocate_empty_array_if_null(snapshots);
                    org_nixos_disnix_disnix_emit_success(params->object, params->arg_pid, (const gchar **)snapshots);
                }
                else
                    org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);

                g_strfreev(snapshots);
            }
        }
        else
        {
            dprintf(log_fd, "Error with creating pipe!\n");
            org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
        }
        
        close(log_fd);
    }
    
    /* Cleanup */
    g_free(params->arg_container);
    g_free(params->arg_component);
    g_free(params);
    
    return NULL;
}

gboolean on_handle_query_latest_snapshot(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_container, const gchar *arg_component)
{
    GThread *thread;
    
    QueryLatestSnapshotParams *params = (QueryLatestSnapshotParams*)g_malloc(sizeof(QueryLatestSnapshotParams));
    params->object = object;
    params->arg_pid = arg_pid;
    params->arg_container = g_strdup(arg_container);
    params->arg_component = g_strdup(arg_component);

    thread = g_thread_new("query-latest-snapshot", disnix_query_latest_snapshot_thread_func, params);
    org_nixos_disnix_disnix_complete_query_latest_snapshot(object, invocation);
    g_thread_unref(thread);
    return TRUE;
}

/* Query missing snapshots method */

typedef struct
{
    OrgNixosDisnixDisnix *object;
    gint arg_pid;
    gchar **arg_component;
}
PrintMissingSnapshotsParams;

static gpointer disnix_print_missing_snapshots_thread_func(gpointer data)
{
    PrintMissingSnapshotsParams *params = (PrintMissingSnapshotsParams*)data;
    int log_fd = open_log_file(params->arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
    }
    else
    {
        /* Declarations */
        int pipefd[2];
        
        /* Print log entry */
        dprintf(log_fd, "Print missing snapshots: ");
        print_paths(log_fd, params->arg_component);
        dprintf(log_fd, "\n");
        
        /* Execute command */
        
        if(pipe(pipefd) == 0)
        {
            int status = fork();

            if(status == -1)
            {
                dprintf(log_fd, "Error with forking dysnomia-snapshots process!\n");
                close(pipefd[0]);
                close(pipefd[1]);
                org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
            }
            else if(status == 0)
            {
                unsigned int i, component_size = g_strv_length(params->arg_component);
                gchar **args = (gchar**)g_malloc((component_size + 3) * sizeof(gchar*));

                args[0] = "dysnomia-snapshots";
                args[1] = "--print-missing";

                for(i = 0; i < component_size; i++)
                    args[i + 2] = params->arg_component[i];
                
                args[i + 2] = NULL;

                close(pipefd[0]); /* Close read-end of pipe */

                dup2(pipefd[1], 1);
                dup2(log_fd, 2);
                execvp("dysnomia-snapshots", args);
               _exit(1);
            }
            else
            {
                char line[BUFFER_SIZE];
                ssize_t line_size;
                gchar **snapshots = NULL;

                close(pipefd[1]); /* Close write-end of pipe */

                while((line_size = read(pipefd[0], line, BUFFER_SIZE - 1)) > 0)
                {
                    line[line_size] = '\0';
                    snapshots = update_lines_vector(snapshots, line);
                }

                close(pipefd[0]);

                wait(&status);

                if(WEXITSTATUS(status) == 0)
                {
                    snapshots = allocate_empty_array_if_null(snapshots);
                    org_nixos_disnix_disnix_emit_success(params->object, params->arg_pid, (const gchar **)snapshots);
                }
                else
                    org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);

                g_strfreev(snapshots);
           }
        }
        else
        {
            dprintf(log_fd, "Error with creating pipe!\n");
            org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
        }
        
        close(log_fd);
    }
    
    /* Cleanup */
    g_strfreev(params->arg_component);
    g_free(params);
    return NULL;
}

gboolean on_handle_print_missing_snapshots(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *const *arg_component)
{
    GThread *thread;
    
    PrintMissingSnapshotsParams *params = (PrintMissingSnapshotsParams*)g_malloc(sizeof(PrintMissingSnapshotsParams));
    params->object = object;
    params->arg_pid = arg_pid;
    params->arg_component = g_strdupv((gchar**)arg_component);

    thread = g_thread_new("print-missing-snapshots", disnix_print_missing_snapshots_thread_func, params);
    org_nixos_disnix_disnix_complete_print_missing_snapshots(object, invocation);
    g_thread_unref(thread);
    return TRUE;
}

/* Import snapshots operation */

typedef struct
{
    OrgNixosDisnixDisnix *object;
    gint arg_pid;
    gchar *arg_container;
    gchar *arg_component;
    gchar **arg_snapshots;
}
ImportSnapshotsParams;

static gpointer disnix_import_snapshots_thread_func(gpointer data)
{
    ImportSnapshotsParams *params = (ImportSnapshotsParams*)data;
    int log_fd = open_log_file(params->arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
    }
    else
    {
        /* Print log entry */
        dprintf(log_fd, "Import snapshots: ");
        print_paths(log_fd, params->arg_snapshots);
        dprintf(log_fd, "\n");
        
        /* Execute command */
        
        int status = fork();
        
        if(status == -1)
        {
            dprintf(log_fd, "Error with forking dysnomia-snapshots process!\n");
            org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
        }
        else if(status == 0)
        {
            unsigned int i, snapshots_size = g_strv_length(params->arg_snapshots);
            gchar **args = (gchar**)g_malloc((snapshots_size + 6) * sizeof(gchar*));
            
            args[0] = "dysnomia-snapshots";
            args[1] = "--import";
            args[2] = "--container";
            args[3] = params->arg_container;
            args[4] = "--component";
            args[5] = params->arg_component;
            
            for(i = 0; i < snapshots_size; i++)
                args[i + 6] = params->arg_snapshots[i];
            
            args[i + 6] = NULL;
            
            dup2(log_fd, 1);
            dup2(log_fd, 2);
            execvp("dysnomia-snapshots", args);
            _exit(1);
        }
        else
        {
            wait(&status);

            if(WEXITSTATUS(status) == 0)
                org_nixos_disnix_disnix_emit_finish(params->object, params->arg_pid);
            else
                org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
        }
        
        close(log_fd);
    }
    
    /* Cleanup */
    g_free(params->arg_container);
    g_free(params->arg_component);
    g_strfreev(params->arg_snapshots);
    g_free(params);
    return NULL;
}

gboolean on_handle_import_snapshots(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_container, const gchar *arg_component, const gchar *const *arg_snapshots)
{
    GThread *thread;
    
    ImportSnapshotsParams *params = (ImportSnapshotsParams*)g_malloc(sizeof(ImportSnapshotsParams));
    params->object = object;
    params->arg_pid = arg_pid;
    params->arg_container = g_strdup(arg_container);
    params->arg_component = g_strdup(arg_component);
    params->arg_snapshots = g_strdupv((gchar**)arg_snapshots);
    
    thread = g_thread_new("import-snapshots", disnix_import_snapshots_thread_func, params);
    org_nixos_disnix_disnix_complete_import_snapshots(object, invocation);
    g_thread_unref(thread);
    return TRUE;
}

/* Resolve snapshots operation */

typedef struct
{
    OrgNixosDisnixDisnix *object;
    gint arg_pid;
    gchar **arg_snapshots;
}
ResolveSnapshotsParams;

static gpointer disnix_resolve_snapshots_thread_func(gpointer data)
{
    ResolveSnapshotsParams *params = (ResolveSnapshotsParams*)data;
    int log_fd = open_log_file(params->arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
    }
    else
    {
        /* Declarations */
        int pipefd[2];
        
        /* Print log entry */
        dprintf(log_fd, "Resolve snapshots: ");
        print_paths(log_fd, params->arg_snapshots);
        dprintf(log_fd, "\n");
        
        /* Execute command */
        
        if(pipe(pipefd) == 0)
        {
            int status = fork();

            if(status == -1)
            {
                dprintf(log_fd, "Error with forking dysnomia-snapshots process!\n");
                close(pipefd[0]);
                close(pipefd[1]);
                org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
            }
            else if(status == 0)
            {
                unsigned int i, snapshots_size = g_strv_length(params->arg_snapshots);
                gchar **args = (gchar**)g_malloc((snapshots_size + 3) * sizeof(gchar*));

                args[0] = "dysnomia-snapshots";
                args[1] = "--resolve";
                
                for(i = 0; i < snapshots_size; i++)
                    args[i + 2] = params->arg_snapshots[i];

                args[i + 2] = NULL;

                close(pipefd[0]); /* Close read-end of pipe */

                dup2(pipefd[1], 1);
                dup2(log_fd, 2);
                execvp("dysnomia-snapshots", args);
                _exit(1);
            }
            else
            {
                char line[BUFFER_SIZE];
                ssize_t line_size;
                gchar **snapshots = NULL;

                close(pipefd[1]); /* Close write-end of pipe */

                while((line_size = read(pipefd[0], line, BUFFER_SIZE - 1)) > 0)
                {
                    line[line_size] = '\0';
                    snapshots = update_lines_vector(snapshots, line);
                }

                close(pipefd[0]);

                wait(&status);
                
                if(WEXITSTATUS(status) == 0)
                {
                    snapshots = allocate_empty_array_if_null(snapshots);
                    org_nixos_disnix_disnix_emit_success(params->object, params->arg_pid, (const gchar **)snapshots);
                }
                else
                    org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);

               g_strfreev(snapshots);
           }
        }
        else
        {
            dprintf(log_fd, "Error with creating pipe!\n");
            org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
        }
    
        close(log_fd);
    }
    
    /* Cleanup */
    g_strfreev(params->arg_snapshots);
    g_free(params);
    return NULL;
}

gboolean on_handle_resolve_snapshots(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *const *arg_snapshots)
{
    GThread *thread;
    
    ResolveSnapshotsParams *params = (ResolveSnapshotsParams*)g_malloc(sizeof(ResolveSnapshotsParams));
    params->object = object;
    params->arg_pid = arg_pid;
    params->arg_snapshots = g_strdupv((gchar**)arg_snapshots);
    
    thread = g_thread_new("resolve-snapshots", disnix_resolve_snapshots_thread_func, params);
    org_nixos_disnix_disnix_complete_resolve_snapshots(object, invocation);
    g_thread_unref(thread);
    return TRUE;
}

/* Clean snapshots method */

typedef struct
{
    OrgNixosDisnixDisnix *object;
    gint arg_pid;
    gint arg_keep;
}
CleanSnapshotsParams;

static gpointer disnix_clean_snapshots_thread_func(gpointer data)
{
    CleanSnapshotsParams *params = (CleanSnapshotsParams*)data;
    int log_fd = open_log_file(params->arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
    }
    else
    {
        /* Declarations */
        int status;
        char keepStr[15];
        
        /* Convert keep value to string */
        sprintf(keepStr, "%d", params->arg_keep);
        
        /* Print log entry */
        dprintf(log_fd, "Clean old snapshots, num of generations to keep: %s!\n", keepStr);
        
        /* Execute command */
    
        status = fork();
    
        if(status == -1)
        {
            dprintf(log_fd, "Error with forking garbage collect process!\n");
            org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
        }
        else if(status == 0)
        {
            char *args[] = {"dysnomia-snapshots", "--gc", "--keep", keepStr, NULL};
            dup2(log_fd, 1);
            dup2(log_fd, 2);
            execvp("dysnomia-snapshots", args);
            dprintf(log_fd, "Error with executing clean snapshots process\n");
            _exit(1);
        }
        else
        {
            wait(&status);

            if(WEXITSTATUS(status) == 0)
                org_nixos_disnix_disnix_emit_finish(params->object, params->arg_pid);
            else
                org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
        }
        
        close(log_fd);
    }
    
    /* Cleanup */
    g_free(params);
    return NULL;
}

gboolean on_handle_clean_snapshots(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, gint arg_keep)
{
    GThread *thread;
    
    CleanSnapshotsParams *params = (CleanSnapshotsParams*)g_malloc(sizeof(CleanSnapshotsParams));
    params->object = object;
    params->arg_pid = arg_pid;
    params->arg_keep = arg_keep;
    
    thread = g_thread_new("clean-snapshots", disnix_clean_snapshots_thread_func, params);
    org_nixos_disnix_disnix_complete_clean_snapshots(object, invocation);
    g_thread_unref(thread);
    return TRUE;
}

/* Delete state operation */

gboolean on_handle_delete_state(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_derivation, const gchar *arg_type, const gchar *const *arg_arguments)
{
    ActivityParams *params = create_activity_params(object, "collect-garbage", arg_pid, arg_derivation, arg_type, arg_arguments);
    GThread *thread = g_thread_new("delete-state", disnix_dysnomia_activity_thread_func, params);
    org_nixos_disnix_disnix_complete_delete_state(object, invocation);
    g_thread_unref(thread);
    return TRUE;
}

/* Get logdir operation */
gboolean on_handle_get_logdir(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation)
{
    org_nixos_disnix_disnix_complete_get_logdir(object, invocation, logdir);
    return TRUE;
}
