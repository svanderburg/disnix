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

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <glib.h>
#include "signals.h"
#include "job.h"
#define BUFFER_SIZE 1024

extern char *activation_modules_dir;

extern GHashTable *job_table;

static int job_counter = 0;

static gchar *generate_derivations_string(gchar **derivation, char *separator)
{
    unsigned int i;
    gchar *derivations_string = g_strdup("");
    
    for(i = 0; i < g_strv_length(derivation); i++)
    {
	gchar *old_derivations_string = derivations_string;
	derivations_string = g_strconcat(old_derivations_string, separator, derivation[i], NULL);
	g_free(old_derivations_string);
    }
    
    return derivations_string;
}

static gint *generate_int_key(gint *key)
{
    gint *ret = (gint*)g_malloc(sizeof(gint));
    *ret = *key;
    return ret;
}

/* Import method */

typedef struct
{
    gchar *closure;
    gint pid;
    DisnixObject *object;
}
DisnixImportParams;

static void disnix_import_thread_func(gpointer data)
{
    /* Declarations */
    gchar *closure;
    gint pid;
    DisnixImportParams *params;
    int closure_fd;
    
    /* Import variables */
    params = (DisnixImportParams*)data;
    closure = params->closure;
    pid = params->pid;

    /* Print log entry */
    g_print("Importing: %s\n", closure);
    
    /* Execute command */
    
    closure_fd = open(closure, O_RDONLY);
    
    if(closure_fd == -1)
    {
	g_printerr("Cannot open closure file!\n");
	disnix_emit_failure_signal(params->object, pid);
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
	    disnix_emit_failure_signal(params->object, pid);
	}
	else
	{
	    wait(&status);
	
	    if(WEXITSTATUS(status) == 0)
		disnix_emit_finish_signal(params->object, pid);
	    else
		disnix_emit_failure_signal(params->object, pid);
	}
    
	close(closure_fd);
    }
        
    /* Free variables */
    g_free(params);
    g_free(closure);
}

gboolean disnix_import(DisnixObject *object, gchar *closure, gint *pid, GError **error)
{
    /* Declarations */
    DisnixImportParams *params;
    Job *job;
    
    /* State object should not be NULL */
    g_assert(object != NULL);

    /* Generate process id */    
    object->pid = job_counter++;
    *pid = object->pid;
    
    /* Create parameter struct */
    params = (DisnixImportParams*)g_malloc(sizeof(DisnixImportParams));
    params->closure = g_strdup(closure);
    params->pid = object->pid;
    params->object = object;
    
    /* Add this call to the job table */
    job = new_job(OP_IMPORT, params, error);
    g_hash_table_insert(job_table, generate_int_key(pid), job);    
    
    return TRUE;
}

/* Export method */

typedef struct
{
    gchar **derivation;
    gint pid;
    DisnixObject *object;
}
DisnixExportParams;

static void disnix_export_thread_func(gpointer data)
{
    /* Declarations */
    gchar **derivation, *derivations_string;
    gint pid;
    char line[BUFFER_SIZE];
    char tempfilename[19] = "/tmp/disnix.XXXXXX";
    DisnixExportParams *params;
    int closure_fd;
    
    /* Import variables */
    params = (DisnixExportParams*)data;
    derivation = params->derivation;
    pid = params->pid;

    /* Generate derivations string */
    derivations_string = generate_derivations_string(derivation, " ");
    
    /* Print log entry */
    g_print("Exporting: %s\n", derivations_string);
    
    /* Execute command */
        
    closure_fd = mkstemp(tempfilename);
    
    if(closure_fd == -1)
    {
	g_printerr("Error opening tempfile!\n");
	disnix_emit_failure_signal(params->object, pid);
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
	    disnix_emit_failure_signal(params->object, pid);
	}
	else
	{
	    wait(&status);
	    
	    if(WEXITSTATUS(status) == 0)
	    {
		gchar *tempfilepaths[2];
		tempfilepaths[0] = tempfilename;
		tempfilepaths[1] = NULL;
		disnix_emit_success_signal(params->object, pid, tempfilepaths);
	    }
	    else
		disnix_emit_failure_signal(params->object, pid);
	}
	
	close(closure_fd);
    }
    
    /* Free variables */    
    g_free(params);
    g_free(derivations_string);
    g_strfreev(derivation);
}

gboolean disnix_export(DisnixObject *object, gchar **derivation, gint *pid, GError **error)
{
    /* Declarations */
    DisnixExportParams *params;
    Job *job;
    
    /* State object should not be NULL */
    g_assert(object != NULL);

    /* Generate process id */    
    object->pid = job_counter++;
    *pid = object->pid;
    
    /* Create parameter struct */
    params = (DisnixExportParams*)g_malloc(sizeof(DisnixExportParams));
    params->derivation = g_strdupv(derivation);
    params->pid = object->pid;
    params->object = object;
    
    /* Add this call to the job table */
    job = new_job(OP_EXPORT, params, error);
    g_hash_table_insert(job_table, generate_int_key(pid), job);
    
    return TRUE;
}

/* Print invalid paths method */

typedef struct
{
    gchar **derivation;
    gint pid;
    DisnixObject *object;
}
DisnixPrintInvalidPathsParams;

static void disnix_print_invalid_thread_func(gpointer data)
{
    /* Declarations */
    gchar **derivation, *derivations_string, *cmd;
    gint pid;    
    DisnixPrintInvalidPathsParams *params;
    int pipefd[2];
    
    /* Import variables */
    params = (DisnixPrintInvalidPathsParams*)data;
    derivation = params->derivation;
    pid = params->pid;
    
    /* Generate derivations string */
    derivations_string = generate_derivations_string(derivation, " ");
    
    /* Print log entry */
    g_print("Print invalid: %s\n", derivations_string);
    
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
	    disnix_emit_failure_signal(params->object, pid);
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
	        puts(line);
	        missing_paths = (gchar**)g_realloc(missing_paths, (missing_paths_size + 1) * sizeof(gchar*));
	        missing_paths[missing_paths_size] = g_strdup(line);	    
	        missing_paths_size++;
	    }
		
	    /* Add NULL value to the end of the list */
	    missing_paths = (gchar**)g_realloc(missing_paths, (missing_paths_size + 1) * sizeof(gchar*));
	    missing_paths[missing_paths_size] = NULL;
	
	    close(pipefd[0]);
	    
	    wait(&status);
	
	    if(WEXITSTATUS(status) == 0)
		disnix_emit_success_signal(params->object, pid, missing_paths);
	    else
		disnix_emit_failure_signal(params->object, pid);
	    
	    g_strfreev(missing_paths);
	}
    }
    else
    {
	fprintf(stderr, "Error with creating a pipe!\n");
	disnix_emit_failure_signal(params->object, pid);
    }
        
    /* Free variables */
    g_free(derivations_string);
    g_free(params);
    g_free(cmd);    
}

gboolean disnix_print_invalid(DisnixObject *object, gchar **derivation, gint *pid, GError **error)
{
    /* Declarations */
    DisnixPrintInvalidPathsParams *params;
    Job *job;
    
    /* State object should not be NULL */
    g_assert(object != NULL);

    /* Generate process id */    
    object->pid = job_counter++;
    *pid = object->pid;
    
    /* Create parameter struct */
    params = (DisnixPrintInvalidPathsParams*)g_malloc(sizeof(DisnixPrintInvalidPathsParams));
    params->derivation = g_strdupv(derivation);
    params->pid = object->pid;
    params->object = object;

    /* Add this call to the job table */
    job = new_job(OP_PRINT_INVALID, params, error);
    g_hash_table_insert(job_table, generate_int_key(pid), job);
        
    return TRUE;
}

/* Realise method */

typedef struct
{
    gchar **derivation;
    gint pid;
    DisnixObject *object;
}
DisnixRealiseParams;

static void disnix_realise_thread_func(gpointer data)
{
    /* Declarations */
    gchar *cmd, **derivation, *derivations_string;
    gint pid;
    DisnixRealiseParams *params;
    int pipefd[2];
    
    /* Import variables */
    params = (DisnixRealiseParams*)data;
    derivation = params->derivation;
    pid = params->pid;

    /* Generate derivation strings */
    derivations_string = generate_derivations_string(derivation, " ");
    
    /* Print log entry */
    g_print("Realising: %s\n", derivations_string);
    
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
	    disnix_emit_failure_signal(params->object, pid);
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
	        realised = (gchar**)g_realloc(realised, (realised_size + 1) * sizeof(gchar*));
	        realised[realised_size] = g_strdup(line);
	        realised_size++;
	    }

	    realised = (gchar**)g_realloc(realised, (realised_size + 1) * sizeof(gchar*));
	    realised[realised_size] = NULL;
	    realised_size++;
	
	    close(pipefd[0]);
	    
	    wait(&status);
	    
	    if(WEXITSTATUS(status) == 0)
		disnix_emit_success_signal(params->object, pid, realised);
	    else
		disnix_emit_failure_signal(params->object, pid);
	    
	    g_strfreev(realised);
	}
    }
    else
    {
	g_printerr("Error with creating a pipe\n");
	disnix_emit_failure_signal(params->object, pid);
    }
    
    /* Free variables */
    g_free(derivations_string);
    g_strfreev(derivation);
    g_free(params);
    g_free(cmd);
}

gboolean disnix_realise(DisnixObject *object, gchar **derivation, gint *pid, GError **error)
{
    /* Declarations */
    DisnixRealiseParams *params;
    Job *job;
    
    /* State object should not be NULL */
    g_assert(object != NULL);

    /* Generate process id */    
    object->pid = job_counter++;
    *pid = object->pid;

    /* Create parameter struct */
    params = (DisnixRealiseParams*)g_malloc(sizeof(DisnixRealiseParams));
    params->derivation = g_strdupv(derivation);
    params->pid = object->pid;
    params->object = object;
    
    /* Add this call to the job table */
    job = new_job(OP_REALISE, params, error);
    g_hash_table_insert(job_table, generate_int_key(pid), job);
    
    return TRUE;
}

/* Set method */

typedef struct
{
    gchar *profile;
    gchar *derivation;    
    gint pid;
    DisnixObject *object;
}
DisnixSetParams;

static void disnix_set_thread_func(gpointer data)
{
    /* Declarations */
    gchar *cmd, *profile, *derivation, *profile_path;
    gint pid;
    int status;
    char line[BUFFER_SIZE];
    DisnixSetParams *params;
    
    /* Import variables */
    params = (DisnixSetParams*)data;
    profile = params->profile;
    derivation = params->derivation;
    pid = params->pid;
    
    /* Print log entry */
    g_print("Set profile: %s with derivation: %s\n", profile, derivation);
    
    /* Execute command */
    
    mkdir(LOCALSTATEDIR "/nix/profiles/disnix", 0755);
    profile_path = g_strconcat(LOCALSTATEDIR "/nix/profiles/disnix/", profile, NULL);

    status = fork();    
    
    if(status == -1)
    {
	g_printerr("Error with forking nix-env process!\n");
	disnix_emit_failure_signal(params->object, pid);
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
	    disnix_emit_finish_signal(params->object, pid);
	else
	    disnix_emit_failure_signal(params->object, pid);
    }
    
    /* Free variables */
    g_free(profile_path);
    g_free(profile);
    g_free(derivation);
    g_free(params);
    g_free(cmd);
}

gboolean disnix_set(DisnixObject *object, const gchar *profile, const gchar *derivation, gint *pid, GError **error)
{
    /* Declarations */
    FILE *fp;
    char line[BUFFER_SIZE];
    DisnixSetParams *params;
    Job *job;
    
    /* State object should not be NULL */
    g_assert(object != NULL);
    
    /* Generate process id */    
    object->pid = job_counter++;
    *pid = object->pid;
    
    /* Create parameter struct */
    params = (DisnixSetParams*)g_malloc(sizeof(DisnixSetParams));
    params->profile = g_strdup(profile);
    params->derivation = g_strdup(derivation);
    params->pid = object->pid;
    params->object = object;
    
    /* Add this call to the job table */
    job = new_job(OP_SET, params, error);
    g_hash_table_insert(job_table, generate_int_key(pid), job);
    
    return TRUE;
}

/* Query installed method */

typedef struct
{
    gchar *profile;
    gint pid;
    DisnixObject *object;
}
DisnixQueryInstalledParams;

static void disnix_query_installed_thread_func(gpointer data)
{
    /* Declarations */
    gchar *cmd, *profile, **derivation = NULL;
    gint pid;
    unsigned int derivation_size = 0;
    FILE *fp;
    char line[BUFFER_SIZE];
    DisnixQueryInstalledParams *params;
    
    /* Import variables */
    params = (DisnixQueryInstalledParams*)data;
    profile = params->profile;
    pid = params->pid;
    
    /* Print log entry */
    g_print("Query installed derivations from profile: %s\n", profile);
    
    /* Execute command */
    
    cmd = g_strconcat(LOCALSTATEDIR "/nix/profiles/disnix/", profile, "/manifest", NULL);

    fp = fopen(cmd, "r");
    if(fp == NULL)
	disnix_emit_failure_signal(params->object, pid); /* Something went wrong with forking the process */
    else
    {
	int status;
	
	/* Read the output */
	
	while(fgets(line, sizeof(line), fp) != NULL)
	{
	    puts(line);
	    derivation = (gchar**)g_realloc(derivation, (derivation_size + 1) * sizeof(gchar*));
	    derivation[derivation_size] = g_strdup(line);	    
	    derivation_size++;
	}
	
	/* Add NULL value to the end of the list */
	derivation = (gchar**)g_realloc(derivation, (derivation_size + 1) * sizeof(gchar*));
	derivation[derivation_size] = NULL;
	
	fclose(fp);
	
	disnix_emit_success_signal(params->object, pid, derivation);
    }
    
    /* Free variables */
    g_strfreev(derivation);
    g_free(profile);
    g_free(params);
    g_free(cmd);
}

gboolean disnix_query_installed(DisnixObject *object, const gchar *profile, gint *pid, GError **error)
{
    /* Declarations */
    DisnixQueryInstalledParams *params;
    Job *job;
    
    /* State object should not be NULL */
    g_assert(object != NULL);
    
    /* Generate process id */    
    object->pid = job_counter++;
    *pid = object->pid;
    
    /* Create parameter struct */
    params = (DisnixQueryInstalledParams*)g_malloc(sizeof(DisnixQueryInstalledParams));
    params->profile = g_strdup(profile);
    params->pid = object->pid;
    params->object = object;

    /* Add this call to the job table */
    job = new_job(OP_QUERY_INSTALLED, params, error);
    g_hash_table_insert(job_table, generate_int_key(pid), job);
    
    return TRUE;
}

/* Query requisites method */

typedef struct
{
    gchar **derivation;
    gint pid;
    DisnixObject *object;
}
DisnixQueryRequisitesParams;

static void disnix_query_requisites_thread_func(gpointer data)
{
    /* Declarations */
    gchar *cmd, **derivation, *derivations_string;
    gint pid;
    DisnixQueryRequisitesParams *params;
    int pipefd[2];
    
    /* Import variables */
    params = (DisnixQueryRequisitesParams*)data;
    derivation = params->derivation;
    pid = params->pid;
    
    /* Generate derivations string */
    derivations_string = generate_derivations_string(derivation, " ");

    /* Print log entry */
    g_print("Query requisites from derivations: %s\n", derivations_string);
    
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
	    disnix_emit_failure_signal(params->object, pid);
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
	        requisites = (gchar**)g_realloc(requisites, (requisites_size + 1) * sizeof(gchar*));
	        requisites[requisites_size] = g_strdup(line);
	        requisites_size++;
	    }
		
	    /* Add NULL value to the end of the list */
	    requisites = (gchar**)g_realloc(requisites, (requisites_size + 1) * sizeof(gchar*));
	    requisites[requisites_size] = NULL;
	    
	    close(pipefd[0]);

	    wait(&status);
	    
	    if(WEXITSTATUS(status) == 0)
		disnix_emit_success_signal(params->object, pid, requisites);
	    else
		disnix_emit_failure_signal(params->object, pid);
	
	    g_strfreev(requisites);
	}
    }
    else
    {
	g_printerr("Error with creating pipe!\n");
	disnix_emit_failure_signal(params->object, pid);
    }
        
    /* Free variables */
    g_free(derivations_string);
    g_free(params);
    g_free(cmd);
}

gboolean disnix_query_requisites(DisnixObject *object, gchar **derivation, gint *pid, GError **error)
{
    /* Declarations */
    DisnixQueryRequisitesParams *params;
    Job *job;
    
    /* State object should not be NULL */
    g_assert(object != NULL);
    
    /* Generate process id */    
    object->pid = job_counter++;
    *pid = object->pid;
    
    /* Create parameter struct */
    params = (DisnixQueryRequisitesParams*)g_malloc(sizeof(DisnixQueryInstalledParams));
    params->derivation = g_strdupv(derivation);
    params->pid = object->pid;
    params->object = object;

    /* Add this call to the job table */
    job = new_job(OP_QUERY_REQUISITES, params, error);
    g_hash_table_insert(job_table, generate_int_key(pid), job);
    
    return TRUE;
}

/* Garbage collect method */

typedef struct
{
    gboolean delete_old;
    gint pid;
    DisnixObject *object;
}
DisnixGarbageCollectParams;

static void disnix_collect_garbage_thread_func(gpointer data)
{
    /* Declarations */
    gchar *cmd;
    gint pid;
    int status;
    gboolean delete_old;
    char line[BUFFER_SIZE];
    DisnixGarbageCollectParams *params;

    /* Import variables */
    params = (DisnixGarbageCollectParams*)data;
    delete_old = params->delete_old;
    pid = params->pid;
    
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
	disnix_emit_failure_signal(params->object, pid);
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
	    disnix_emit_finish_signal(params->object, pid);
	else
	    disnix_emit_failure_signal(params->object, pid);
    }
        
    /* Free variables */
    g_free(params);
    g_free(cmd);    
}

gboolean disnix_collect_garbage(DisnixObject *object, const gboolean delete_old, gint *pid, GError **error)
{
    /* Declarations */
    DisnixGarbageCollectParams *params;
    Job *job;

    /* State object should not be NULL */
    g_assert(object != NULL);

    /* Generate process id */    
    object->pid = job_counter++;
    *pid = object->pid;
    
    /* Create parameter struct */
    params = (DisnixGarbageCollectParams*)g_malloc(sizeof(DisnixGarbageCollectParams));
    params->delete_old = delete_old;
    params->pid = object->pid;
    params->object = object;

    /* Add this call to the job table */
    job = new_job(OP_COLLECT_GARBAGE, params, error);
    g_hash_table_insert(job_table, generate_int_key(pid), job);
    
    return TRUE;
}

/* Activate method */

typedef struct
{
    gchar *derivation;
    gchar *type;
    gchar **arguments;
    gint pid;
    DisnixObject *object;
}
DisnixActivateParams;

static void disnix_activate_thread_func(gpointer data)
{
    /* Declarations */
    gchar *derivation, *type, **arguments, *arguments_string;
    gint pid;
    char line[BUFFER_SIZE];
    DisnixActivateParams *params;
    int status;
        
    /* Import variables */
    params = (DisnixActivateParams*)data;
    derivation = params->derivation;
    type = params->type;
    arguments = params->arguments;
    pid = params->pid;
    
    /* Generate arguments string */
    arguments_string = generate_derivations_string(arguments, " ");
    
    /* Print log entry */
    g_print("Activate: %s of type: %s with arguments: %s\n", derivation, type, arguments_string);
    
    /* Execute command */

    status = fork();
    
    if(status == 0)
    {
	unsigned int i;
	gchar *cmd = g_strconcat(activation_modules_dir, "/", type, NULL);
	char *args[] = {cmd, "activate", derivation, NULL};
	
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
	disnix_emit_failure_signal(params->object, pid);
    }
    else
    {
	wait(&status);
	
	if(WEXITSTATUS(status) == 0)
	    disnix_emit_finish_signal(params->object, pid);
	else
	    disnix_emit_failure_signal(params->object, pid);
    }
    
    /* Free variables */
    g_free(derivation);
    g_free(type);
    g_free(arguments_string);
    g_strfreev(arguments);
    g_free(params);
}

gboolean disnix_activate(DisnixObject *object, const gchar *derivation, const gchar *type, gchar **arguments, gint *pid, GError **error)
{
    /* Declarations */
    DisnixActivateParams *params;
    Job *job;
    
    /* State object should not be NULL */
    g_assert(object != NULL);

    /* Generate process id */    
    object->pid = job_counter++;
    *pid = object->pid;
    
    /* Create parameter struct */
    params = (DisnixActivateParams*)g_malloc(sizeof(DisnixActivateParams));
    params->derivation = g_strdup(derivation);
    params->type = g_strdup(type);
    params->arguments = g_strdupv(arguments);
    params->pid = object->pid;
    params->object = object;

    /* Add this call to the job table */
    job = new_job(OP_ACTIVATE, params, error);
    g_hash_table_insert(job_table, generate_int_key(pid), job);    
    
    return TRUE;    
}

/* Deactivate method */

typedef struct
{
    gchar *derivation;
    gchar *type;
    gchar **arguments;
    gint pid;
    DisnixObject *object;
}
DisnixDeactivateParams;

static void disnix_deactivate_thread_func(gpointer data)
{
    /* Declarations */
    gchar *derivation, *type, **arguments, *arguments_string;
    gint pid;
    char line[BUFFER_SIZE];
    DisnixActivateParams *params;
    int status;
        
    /* Import variables */
    params = (DisnixActivateParams*)data;
    derivation = params->derivation;
    type = params->type;
    arguments = params->arguments;
    pid = params->pid;
    
    /* Generate arguments string */
    arguments_string = generate_derivations_string(arguments, " ");
    
    /* Print log entry */
    g_print("Deactivate: %s of type: %s with arguments: %s\n", derivation, type, arguments_string);
    
    /* Execute command */

    status = fork();
    
    if(status == 0)
    {
	unsigned int i;
	gchar *cmd = g_strconcat(activation_modules_dir, "/", type, NULL);
	char *args[] = {cmd, "deactivate", derivation, NULL};
	
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
	disnix_emit_failure_signal(params->object, pid);
    }
    else
    {
	wait(&status);
	
	if(WEXITSTATUS(status) == 0)
	    disnix_emit_finish_signal(params->object, pid);
	else
	    disnix_emit_failure_signal(params->object, pid);
    }
    
    /* Free variables */
    g_free(derivation);
    g_free(type);
    g_free(arguments_string);
    g_strfreev(arguments);
    g_free(params);
}

gboolean disnix_deactivate(DisnixObject *object, const gchar *derivation, const gchar *type, gchar **arguments, gint *pid, GError **error)
{
    /* Declarations */
    DisnixDeactivateParams *params;
    Job *job;
    
    /* State object should not be NULL */
    g_assert(object != NULL);

    /* Generate process id */    
    object->pid = job_counter++;
    *pid = object->pid;
    
    /* Create parameter struct */
    params = (DisnixDeactivateParams*)g_malloc(sizeof(DisnixDeactivateParams));
    params->derivation = g_strdup(derivation);
    params->type = g_strdup(type);
    params->arguments = g_strdupv(arguments);
    params->pid = object->pid;
    params->object = object;

    /* Add this call to the job table */
    job = new_job(OP_DEACTIVATE, params, error);
    g_hash_table_insert(job_table, generate_int_key(pid), job);    
    
    return TRUE;    
}

gboolean disnix_lock(DisnixObject *object, gint *pid, GError **error)
{
    return TRUE;
}

gboolean disnix_unlock(DisnixObject *object, gint *pid, GError **error)
{
    return TRUE;
}

/* Acknowledge method */

static void print_job_item(gpointer key, gpointer value, gpointer user_data)
{
    gchar *pid = (gchar*)key;
    Job *job = (Job*)value;
    
    g_print("key: %s, job operation: %d\n", pid, job->operation);    
}

gboolean disnix_acknowledge(DisnixObject *object, gint pid, GError **error)
{
    Job *job;
    
    /* State object should not be NULL */
    g_assert(object != NULL);
    
    job = (Job*)g_hash_table_lookup(job_table, &pid);
    
    g_print("Acknowledging PID: %d\n", pid);
    
    if(job != NULL)
    {
	int status;
	
	g_print("Running PID: %d\n", pid);

	job->running = TRUE;
	
	status = fork();
	
	if(status == -1)
	{
	    g_printerr("Error with forking job process!\n");
	    disnix_emit_failure_signal(object, pid);	    
	}
	else if(status == 0)
	{
	    switch(job->operation)
	    {
		case OP_IMPORT:    
		    disnix_import_thread_func(job->params);		
		    break;
		case OP_EXPORT:
		    disnix_export_thread_func(job->params);
		    break;
		case OP_PRINT_INVALID:
		    disnix_print_invalid_thread_func(job->params);
		    break;
		case OP_REALISE:
		    disnix_realise_thread_func(job->params);
		    break;
		case OP_SET:
		    disnix_set_thread_func(job->params);
		    break;
		case OP_QUERY_INSTALLED:
		    disnix_query_installed_thread_func(job->params);
		    break;
		case OP_QUERY_REQUISITES:
		    disnix_query_requisites_thread_func(job->params);
		    break;
		case OP_COLLECT_GARBAGE:
		    disnix_collect_garbage_thread_func(job->params);
		    break;
		case OP_ACTIVATE:
		    disnix_activate_thread_func(job->params);
		    break;
		case OP_DEACTIVATE:
		    disnix_deactivate_thread_func(job->params);
		    break;
		default:
		    g_printerr("Unknown operation: %d\n", job->operation);
	    }
	    
	    _exit(0);
	}	
    }
    
    return TRUE;
}
