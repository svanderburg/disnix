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
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <glib.h>
#include "hash.h"
#include "signals.h"
#include "job.h"
#define BUFFER_SIZE 1024

extern char *activation_modules_dir;

extern GHashTable *job_table;

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

/* Import method */

typedef struct
{
    gchar *closure;
    gchar *pid;
    DisnixObject *object;
}
DisnixImportParams;

static void disnix_import_thread_func(gpointer data)
{
    /* Declarations */
    gchar *closure, *derivations_string, *cmd, *pid;
    FILE *fp;
    char line[BUFFER_SIZE];
    DisnixImportParams *params;
    
    /* Import variables */
    params = (DisnixImportParams*)data;
    closure = params->closure;
    pid = params->pid;

    /* Print log entry */
    g_print("Importing: %s\n", closure);
    
    /* Execute command */
    cmd = g_strconcat("cat ", closure, " | nix-store --import ", NULL);
    
    fp = popen(cmd, "r");
    if(fp == NULL)
	disnix_emit_failure_signal(params->object, pid); /* Something went wrong with forking the process */
    else
    {
	int status;
	
	while(fgets(line, sizeof(line), fp) != NULL)
	    puts(line);
	
	status = pclose(fp);
	
	if(status == -1 || WEXITSTATUS(status) != 0)
	    disnix_emit_failure_signal(params->object, pid);
	else
	    disnix_emit_finish_signal(params->object, pid);
    }
    
    /* Free variables */
    g_free(pid);
    g_free(derivations_string);
    g_free(params);
    g_free(cmd);
    g_free(closure);
}

gboolean disnix_import(DisnixObject *object, gchar *closure, gchar **pid, GError **error)
{
    /* Declarations */
    gchar *pidstring;
    DisnixImportParams *params;
    Job *job;
    
    /* State object should not be NULL */
    g_assert(object != NULL);

    /* Generate process id */    
    pidstring = g_strconcat("import:", closure, NULL);
    object->pid = string_to_hash(pidstring);
    *pid = object->pid;
    g_free(pidstring);
    
    /* Create parameter struct */
    params = (DisnixImportParams*)g_malloc(sizeof(DisnixImportParams));
    params->closure = g_strdup(closure);
    params->pid = g_strdup(object->pid);
    params->object = object;
    
    /* Add this call to the job table */
    job = new_job(OP_IMPORT, params, error);
    g_hash_table_insert(job_table, g_strdup(params->pid), job);    
    
    return TRUE;
}

/* Export method */

typedef struct
{
    gchar **derivation;
    gchar *pid;
    DisnixObject *object;
}
DisnixExportParams;

static void disnix_export_thread_func(gpointer data)
{
    /* Declarations */
    gchar **derivation, *derivations_string, *cmd, *pid;
    FILE *fp;
    char line[BUFFER_SIZE];
    char *tempfilename;
    DisnixExportParams *params;
    
    /* Import variables */
    params = (DisnixExportParams*)data;
    derivation = params->derivation;
    pid = params->pid;

    /* Generate derivation strings */
    derivations_string = generate_derivations_string(derivation, " ");
    
    /* Generate temp file name */
    tempfilename = tempnam("/tmp", "disnix_");
    
    /* Print log entry */
    g_print("Exporting: %s\n", derivations_string);
    
    /* Execute command */
    cmd = g_strconcat("nix-store --export ", derivations_string, " > ", tempfilename, NULL);
    
    fp = popen(cmd, "r");
    if(fp == NULL)
	disnix_emit_failure_signal(params->object, pid); /* Something went wrong with forking the process */
    else
    {
	int status;
	
	while(fgets(line, sizeof(line), fp) != NULL)
	    puts(line);
	
	status = pclose(fp);
	
	if(status == -1 || WEXITSTATUS(status) != 0)
	    disnix_emit_failure_signal(params->object, pid);
	else
	{
	    gchar *tempfilepaths[2];
	    tempfilepaths[0] = tempfilename;
	    tempfilepaths[1] = NULL;
	    disnix_emit_success_signal(params->object, pid, tempfilepaths);
	}
    }
    
    /* Free variables */    
    g_free(pid);
    g_free(derivations_string);
    g_free(params);
    g_free(cmd);
    g_strfreev(derivation);
}

gboolean disnix_export(DisnixObject *object, gchar **derivation, gchar **pid, GError **error)
{
    /* Declarations */
    gchar *pidstring, *derivations_string;
    DisnixExportParams *params;
    Job *job;
    
    /* State object should not be NULL */
    g_assert(object != NULL);

    /* Generate derivations string */
    derivations_string = generate_derivations_string(derivation, ":");

    /* Generate process id */    
    pidstring = g_strconcat("export", derivations_string, NULL);
    object->pid = string_to_hash(pidstring);
    *pid = object->pid;
    g_free(pidstring);
    g_free(derivations_string);
    
    /* Create parameter struct */
    params = (DisnixExportParams*)g_malloc(sizeof(DisnixExportParams));
    params->derivation = g_strdupv(derivation);
    params->pid = g_strdup(object->pid);
    params->object = object;
    
    /* Add this call to the job table */
    job = new_job(OP_EXPORT, params, error);
    g_hash_table_insert(job_table, g_strdup(params->pid), job);
    
    return TRUE;    
}

/* Print invalid paths method */

typedef struct
{
    gchar **derivation;
    gchar *pid;
    DisnixObject *object;
}
DisnixPrintInvalidPathsParams;

static void disnix_print_invalid_thread_func(gpointer data)
{
    /* Declarations */
    gchar **derivation, *derivations_string, *cmd, *pid, **missing_paths = NULL;
    unsigned int missing_paths_size = 0;
    FILE *fp;
    char line[BUFFER_SIZE];
    DisnixPrintInvalidPathsParams *params;

    /* Import variables */
    params = (DisnixPrintInvalidPathsParams*)data;
    derivation = params->derivation;
    pid = params->pid;
    
    /* Generate derivations string */
    derivations_string = generate_derivations_string(derivation, " ");
    
    /* Print log entry */
    g_print("Print invalid: %s\n", derivations_string);
    
    /* Execute command */
    cmd = g_strconcat("nix-store --check-validity --print-invalid ", derivations_string, NULL);
    
    fp = popen(cmd, "r");
    if(fp == NULL)
	disnix_emit_failure_signal(params->object, pid); /* Something went wrong with forking the process */
    else
    {
	int status;
	
	/* Read the output */
	while(fgets(line, sizeof(line), fp) != NULL)
	{
	    puts(line);
	    missing_paths = (gchar**)g_realloc(missing_paths, (missing_paths_size + 1) * sizeof(gchar*));
	    missing_paths[missing_paths_size] = g_strdup(line);	    
	    missing_paths_size++;
	}
	
	/* Add NULL value to the end of the list */
	missing_paths = (gchar**)g_realloc(missing_paths, (missing_paths_size + 1) * sizeof(gchar*));
	missing_paths[missing_paths_size] = NULL;
	
	status = pclose(fp);
	
	if(status == -1 || WEXITSTATUS(status) != 0)
	    disnix_emit_failure_signal(params->object, pid);
	else
	    disnix_emit_success_signal(params->object, pid, missing_paths);
    }
    
    /* Free variables */
    g_strfreev(missing_paths);
    g_free(derivations_string);
    g_free(pid);
    g_free(params);
    g_free(cmd);    
}

gboolean disnix_print_invalid(DisnixObject *object, gchar **derivation, gchar **pid, GError **error)
{
    /* Declarations */
    gchar *pidstring, *derivations_string;
    DisnixPrintInvalidPathsParams *params;
    Job *job;
    
    /* State object should not be NULL */
    g_assert(object != NULL);

    /* Generate derivations string */
    derivations_string = generate_derivations_string(derivation, ":");

    /* Generate process id */
    pidstring = g_strconcat("print_invalid:", derivations_string, NULL);
    object->pid = string_to_hash(pidstring);
    *pid = object->pid;
    g_free(pidstring);
    g_free(derivations_string);
    
    /* Create parameter struct */
    params = (DisnixPrintInvalidPathsParams*)g_malloc(sizeof(DisnixPrintInvalidPathsParams));
    params->derivation = g_strdupv(derivation);
    params->pid = g_strdup(object->pid);
    params->object = object;

    /* Add this call to the job table */
    job = new_job(OP_PRINT_INVALID, params, error);
    g_hash_table_insert(job_table, g_strdup(params->pid), job);
        
    return TRUE;
}

/* Realise method */

typedef struct
{
    gchar **derivation;
    gchar *pid;
    DisnixObject *object;
}
DisnixRealiseParams;

static void disnix_realise_thread_func(gpointer data)
{
    /* Declarations */
    gchar *cmd, **derivation, *derivations_string, **realised = NULL, *pid;
    unsigned int realised_size = 0;
    FILE *fp;
    char line[BUFFER_SIZE];
    DisnixRealiseParams *params;
    
    /* Import variables */
    params = (DisnixRealiseParams*)data;
    derivation = params->derivation;
    pid = params->pid;

    /* Generate derivation strings */
    derivations_string = generate_derivations_string(derivation, " ");
    
    /* Print log entry */
    g_print("Realising: %s\n", derivations_string);
    
    /* Execute command */    
    cmd = g_strconcat("nix-store -r ", derivations_string, NULL);
    
    fp = popen(cmd, "r");
    if(fp == NULL)
	disnix_emit_failure_signal(params->object, pid); /* Something went wrong with forking the process */
    else
    {
	int status;
	
	while(fgets(line, sizeof(line), fp) != NULL)
	{
	    puts(line);
	    realised = (gchar**)g_realloc(realised, (realised_size + 1) * sizeof(gchar*));
	    realised[realised_size] = g_strdup(line);
	    realised_size++;
	}
	
	realised = (gchar**)g_realloc(realised, (realised_size + 1) * sizeof(gchar*));
	realised[realised_size] = NULL;
	realised_size++;
	
	status = pclose(fp);
	
	if(status == -1 || WEXITSTATUS(status) != 0)
	    disnix_emit_failure_signal(params->object, pid);
	else
	    disnix_emit_success_signal(params->object, pid, realised);
    }
    
    /* Free variables */
    g_free(derivations_string);
    g_strfreev(derivation);
    g_strfreev(realised);
    g_free(pid);
    g_free(params);
    g_free(cmd);
}

gboolean disnix_realise(DisnixObject *object, gchar **derivation, gchar **pid, GError **error)
{
    /* Declarations */
    gchar *pidstring;
    DisnixRealiseParams *params;
    Job *job;
    
    /* State object should not be NULL */
    g_assert(object != NULL);

    /* Generate process id */
    pidstring = g_strconcat("realise:", derivation, NULL);
    object->pid = string_to_hash(pidstring);
    *pid = object->pid;
    g_free(pidstring);

    /* Create parameter struct */
    params = (DisnixRealiseParams*)g_malloc(sizeof(DisnixRealiseParams));
    params->derivation = g_strdupv(derivation);
    params->pid = g_strdup(object->pid);
    params->object = object;
    
    /* Add this call to the job table */
    job = new_job(OP_REALISE, params, error);
    g_hash_table_insert(job_table, g_strdup(params->pid), job);
    return TRUE;
}

/* Set method */

typedef struct
{
    gchar *profile;
    gchar *derivation;    
    gchar *pid;
    DisnixObject *object;
}
DisnixSetParams;

static void disnix_set_thread_func(gpointer data)
{
    /* Declarations */
    gchar *cmd, *profile, *derivation, *pid;
    FILE *fp;
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
    cmd = g_strconcat("nix-env -p ", LOCALSTATEDIR, "/nix/profiles/disnix/", profile, " --set ", derivation, NULL);

    fp = popen(cmd, "r");
    if(fp == NULL)
	disnix_emit_failure_signal(params->object, pid); /* Something went wrong with forking the process */
    else
    {
	int status;
	
	while(fgets(line, sizeof(line), fp) != NULL)
	    puts(line);
	
	status = pclose(fp);
	
	if(status == -1 || WEXITSTATUS(status) != 0)
	    disnix_emit_failure_signal(params->object, pid);
	else
	    disnix_emit_finish_signal(params->object, pid);
    }
    
    /* Free variables */
    g_free(profile);
    g_free(derivation);
    g_free(pid);
    g_free(params);
    g_free(cmd);
}

gboolean disnix_set(DisnixObject *object, const gchar *profile, const gchar *derivation, gchar **pid, GError **error)
{
    /* Declarations */
    gchar *pidstring;
    FILE *fp;
    char line[BUFFER_SIZE];
    DisnixSetParams *params;
    Job *job;
    
    /* State object should not be NULL */
    g_assert(object != NULL);
    
    /* Generate process id */
    pidstring = g_strconcat("set:", profile, ":", derivation, NULL);
    object->pid = string_to_hash(pidstring);
    *pid = object->pid;
    g_free(pidstring);
    
    /* Create parameter struct */
    params = (DisnixSetParams*)g_malloc(sizeof(DisnixSetParams));
    params->profile = g_strdup(profile);
    params->derivation = g_strdup(derivation);
    params->pid = g_strdup(object->pid);
    params->object = object;
    
    /* Add this call to the job table */
    job = new_job(OP_SET, params, error);
    g_hash_table_insert(job_table, g_strdup(params->pid), job);
    
    return TRUE;
}

/* Query installed method */

typedef struct
{
    gchar *profile;
    gchar *pid;
    DisnixObject *object;
}
DisnixQueryInstalledParams;

static void disnix_query_installed_thread_func(gpointer data)
{
    /* Declarations */
    gchar *cmd, *profile, *pid, **derivation = NULL;
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
    
    cmd = g_strconcat("nix-env -q \'*\' --out-path --no-name -p ", LOCALSTATEDIR, "/nix/profiles/disnix/", profile, NULL);

    fp = popen(cmd, "r");
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
	
	status = pclose(fp);
	
	if(status == -1 || WEXITSTATUS(status) != 0)
	    disnix_emit_failure_signal(params->object, pid);
	else
	    disnix_emit_success_signal(params->object, pid, derivation);
    }
    
    /* Free variables */
    g_strfreev(derivation);
    g_free(profile);
    g_free(pid);
    g_free(params);
    g_free(cmd);
}

gboolean disnix_query_installed(DisnixObject *object, const gchar *profile, gchar **pid, GError **error)
{
    /* Declarations */
    gchar *pidstring;
    DisnixQueryInstalledParams *params;
    Job *job;
    
    /* State object should not be NULL */
    g_assert(object != NULL);
    
    /* Generate process id */
    pidstring = g_strconcat("query_installed:", profile, NULL);
    object->pid = string_to_hash(pidstring);
    *pid = object->pid;
    g_free(pidstring);
    
    /* Create parameter struct */
    params = (DisnixQueryInstalledParams*)g_malloc(sizeof(DisnixQueryInstalledParams));
    params->profile = g_strdup(profile);
    params->pid = g_strdup(object->pid);
    params->object = object;

    /* Add this call to the job table */
    job = new_job(OP_QUERY_INSTALLED, params, error);
    g_hash_table_insert(job_table, g_strdup(params->pid), job);
    
    return TRUE;
}

/* Query requisites method */

typedef struct
{
    gchar **derivation;
    gchar *pid;
    DisnixObject *object;
}
DisnixQueryRequisitesParams;

static void disnix_query_requisites_thread_func(gpointer data)
{
    /* Declarations */
    gchar *cmd, *pid, **derivation, *derivations_string, **requisites = NULL;
    unsigned int requisites_size = 0;
    FILE *fp;
    char line[BUFFER_SIZE];
    DisnixQueryRequisitesParams *params;
    
    /* Import variables */
    params = (DisnixQueryRequisitesParams*)data;
    derivation = params->derivation;
    pid = params->pid;
    
    /* Generate derivations string */
    derivations_string = generate_derivations_string(derivation, " ");

    /* Print log entry */
    g_print("Query requisites from derivations: %s\n", derivations_string);
    
    /* Execute command */
    
    cmd = g_strconcat("nix-store -qR ", derivations_string, NULL);

    fp = popen(cmd, "r");
    if(fp == NULL)
	disnix_emit_failure_signal(params->object, pid); /* Something went wrong with forking the process */
    else
    {
	int status;
	
	/* Read the output */
	
	while(fgets(line, sizeof(line), fp) != NULL)
	{
	    puts(line);
	    requisites = (gchar**)g_realloc(requisites, (requisites_size + 1) * sizeof(gchar*));
	    requisites[requisites_size] = g_strdup(line);
	    requisites_size++;
	}
	
	/* Add NULL value to the end of the list */
	requisites = (gchar**)g_realloc(requisites, (requisites_size + 1) * sizeof(gchar*));
	requisites[requisites_size] = NULL;
	
	status = pclose(fp);
	
	if(status == -1 || WEXITSTATUS(status) != 0)
	    disnix_emit_failure_signal(params->object, pid);
	else
	    disnix_emit_success_signal(params->object, pid, requisites);
    }
    
    /* Free variables */
    g_strfreev(requisites);
    g_free(derivations_string);
    g_free(pid);
    g_free(params);
    g_free(cmd);
}

gboolean disnix_query_requisites(DisnixObject *object, gchar **derivation, gchar **pid, GError **error)
{
    /* Declarations */
    gchar *pidstring, *derivations_string;
    DisnixQueryRequisitesParams *params;
    Job *job;
    
    /* State object should not be NULL */
    g_assert(object != NULL);
    
    /* Generate derivations string */
    derivations_string = generate_derivations_string(derivation, ":");

    /* Generate process id */
    pidstring = g_strconcat("query_requisites:", derivations_string, NULL);
    object->pid = string_to_hash(pidstring);
    *pid = object->pid;
    g_free(pidstring);
    g_free(derivations_string);
    
    /* Create parameter struct */
    params = (DisnixQueryRequisitesParams*)g_malloc(sizeof(DisnixQueryInstalledParams));
    params->derivation = g_strdupv(derivation);
    params->pid = g_strdup(object->pid);
    params->object = object;

    /* Add this call to the job table */
    job = new_job(OP_QUERY_REQUISITES, params, error);
    g_hash_table_insert(job_table, g_strdup(params->pid), job);
    
    return TRUE;
}

/* Garbage collect method */

typedef struct
{
    gboolean delete_old;
    gchar *pid;
    DisnixObject *object;
}
DisnixGarbageCollectParams;

static void disnix_collect_garbage_thread_func(gpointer data)
{
    /* Declarations */
    gchar *cmd, *pid;
    gboolean delete_old;
    FILE *fp;
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
    if(delete_old)
	cmd = g_strdup("nix-collect-garbage -d");
    else
	cmd = g_strdup("nix-collect-garbage");
	
    fp = popen(cmd, "r");
    if(fp == NULL)
	disnix_emit_failure_signal(params->object, pid); /* Something went wrong with forking the process */
    else
    {
	int status;
	
	/* Read the output */
	while(fgets(line, sizeof(line), fp) != NULL)
	    puts(line);
	
	status = pclose(fp);
	
	if(status == -1 || WEXITSTATUS(status) != 0)
	    disnix_emit_failure_signal(params->object, pid);
	else
	    disnix_emit_finish_signal(params->object, pid);
    }
    
    /* Free variables */
    g_free(pid);
    g_free(params);
    g_free(cmd);    
}

gboolean disnix_collect_garbage(DisnixObject *object, const gboolean delete_old, gchar **pid, GError **error)
{
    /* Declarations */
    gchar *pidstring;
    DisnixGarbageCollectParams *params;
    Job *job;

    /* State object should not be NULL */
    g_assert(object != NULL);

    /* Generate process id */
    if(delete_old)
	pidstring = g_strdup("collect_garbage:delete_old");
    else
	pidstring = g_strdup("collect_garbage");
	
    object->pid = string_to_hash(pidstring);
    *pid = object->pid;
    g_free(pidstring);
    
    /* Create parameter struct */
    params = (DisnixGarbageCollectParams*)g_malloc(sizeof(DisnixGarbageCollectParams));
    params->delete_old = delete_old;
    params->pid = g_strdup(object->pid);
    params->object = object;

    /* Add this call to the job table */
    job = new_job(OP_COLLECT_GARBAGE, params, error);
    g_hash_table_insert(job_table, g_strdup(params->pid), job);    
    return TRUE;
}

/* Activate method */

typedef struct
{
    gchar *derivation;
    gchar *type;
    gchar **arguments;
    gchar *pid;
    DisnixObject *object;
}
DisnixActivateParams;

static void disnix_activate_thread_func(gpointer data)
{
    /* Declarations */
    gchar *derivation, *type, **arguments, *arguments_string, *cmd, *pid;
    FILE *fp;
    char line[BUFFER_SIZE];
    DisnixActivateParams *params;

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

    cmd = g_strconcat("env ", arguments_string, " ", activation_modules_dir, "/", type, " activate ", derivation, NULL);
    
    fp = popen(cmd, "r");
    if(fp == NULL)
        disnix_emit_failure_signal(params->object, pid); /* Something went wrong with forking the process */
    else
    {
	int status;
	
        /* Read the output */
        while(fgets(line, sizeof(line), fp) != NULL)
    	    puts(line);
	
	status = pclose(fp);
	
	if(status == -1 || WEXITSTATUS(status) != 0)
	    disnix_emit_failure_signal(params->object, pid);
	else
	    disnix_emit_finish_signal(params->object, pid);
    }

    /* Free variables */
    g_free(derivation);
    g_free(type);
    g_free(arguments_string);
    g_strfreev(arguments);
    g_free(pid);
    g_free(params);
    g_free(cmd);
}

gboolean disnix_activate(DisnixObject *object, const gchar *derivation, const gchar *type, gchar **arguments, gchar **pid, GError **error)
{
    /* Declarations */
    gchar *pidstring, *arguments_string;
    DisnixActivateParams *params;
    Job *job;
    
    /* State object should not be NULL */
    g_assert(object != NULL);

    /* Generate arguments string */
    arguments_string = generate_derivations_string(arguments, ":");

    /* Generate process id */    
    pidstring = g_strconcat("activate:", derivation, ":", type, ":", arguments_string, NULL);
    object->pid = string_to_hash(pidstring);
    *pid = object->pid;    
    g_free(pidstring);
    g_free(arguments_string);
    
    /* Create parameter struct */
    params = (DisnixActivateParams*)g_malloc(sizeof(DisnixActivateParams));
    params->derivation = g_strdup(derivation);
    params->type = g_strdup(type);
    params->arguments = g_strdupv(arguments);
    params->pid = g_strdup(object->pid);
    params->object = object;

    /* Add this call to the job table */
    job = new_job(OP_ACTIVATE, params, error);
    g_hash_table_insert(job_table, g_strdup(params->pid), job);    
    
    return TRUE;    
}

/* Deactivate method */

typedef struct
{
    gchar *derivation;
    gchar *type;
    gchar **arguments;
    gchar *pid;
    DisnixObject *object;
}
DisnixDeactivateParams;

static void disnix_deactivate_thread_func(gpointer data)
{
    /* Declarations */
    gchar *derivation, *type, **arguments, *arguments_string, *cmd, *pid;
    FILE *fp;
    char line[BUFFER_SIZE];
    DisnixDeactivateParams *params;

    /* Import variables */
    params = (DisnixDeactivateParams*)data;
    derivation = params->derivation;
    type = params->type;
    arguments = params->arguments;
    pid = params->pid;
    
    /* Generate arguments string */
    arguments_string = generate_derivations_string(arguments, " ");
    
    /* Print log entry */
    g_print("Deactivate: %s of type: %s with arguments: %s\n", derivation, type, arguments_string);
    
    /* Execute command */

    cmd = g_strconcat("env ", arguments_string, " ", activation_modules_dir, "/", type, " deactivate ", derivation, NULL);
    
    fp = popen(cmd, "r");
    if(fp == NULL)
        disnix_emit_failure_signal(params->object, pid); /* Something went wrong with forking the process */
    else
    {
	int status;
	
        /* Read the output */
        while(fgets(line, sizeof(line), fp) != NULL)
    	    puts(line);
	
	status = pclose(fp);
	
	if(status == -1 || WEXITSTATUS(status) != 0)
	    disnix_emit_failure_signal(params->object, pid);
	else
	    disnix_emit_finish_signal(params->object, pid);
    }

    /* Free variables */
    g_free(derivation);
    g_free(type);
    g_free(arguments_string);
    g_strfreev(arguments);
    g_free(pid);
    g_free(params);
    g_free(cmd);
}

gboolean disnix_deactivate(DisnixObject *object, const gchar *derivation, const gchar *type, gchar **arguments, gchar **pid, GError **error)
{
    /* Declarations */
    gchar *pidstring, *arguments_string;
    DisnixDeactivateParams *params;
    Job *job;
    
    /* State object should not be NULL */
    g_assert(object != NULL);

    /* Generate arguments string */
    arguments_string = generate_derivations_string(arguments, ":");

    /* Generate process id */    
    pidstring = g_strconcat("deactivate:", derivation, ":", type, ":", arguments_string, NULL);
    object->pid = string_to_hash(pidstring);
    *pid = object->pid;    
    g_free(pidstring);
    g_free(arguments_string);
    
    /* Create parameter struct */
    params = (DisnixDeactivateParams*)g_malloc(sizeof(DisnixDeactivateParams));
    params->derivation = g_strdup(derivation);
    params->type = g_strdup(type);
    params->arguments = g_strdupv(arguments);
    params->pid = g_strdup(object->pid);
    params->object = object;

    /* Add this call to the job table */
    job = new_job(OP_DEACTIVATE, params, error);
    g_hash_table_insert(job_table, g_strdup(params->pid), job);    
    
    return TRUE;    
}

gboolean disnix_lock(DisnixObject *object, gchar **pid, GError **error)
{
    return TRUE;
}

gboolean disnix_unlock(DisnixObject *object, gchar **pid, GError **error)
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

gboolean disnix_acknowledge(DisnixObject *object, gchar *pid, GError **error)
{
    Job *job;
    
    /* State object should not be NULL */
    g_assert(object != NULL);
    
    job = (Job*)g_hash_table_lookup(job_table, pid);
    
    g_print("Acknowledging PID: %s\n", pid);
    
    if(job != NULL)
    {
	job->running = TRUE;
	
	switch(job->operation)
	{
	    case OP_IMPORT:
		g_thread_create((GThreadFunc)disnix_import_thread_func, job->params, FALSE, job->error);
		break;
	    case OP_EXPORT:
		g_thread_create((GThreadFunc)disnix_export_thread_func, job->params, FALSE, job->error);
		break;
	    case OP_PRINT_INVALID:
		g_thread_create((GThreadFunc)disnix_print_invalid_thread_func, job->params, FALSE, job->error);
		break;
	    case OP_REALISE:
		g_thread_create((GThreadFunc)disnix_realise_thread_func, job->params, FALSE, job->error);
		break;
	    case OP_SET:
		g_thread_create((GThreadFunc)disnix_set_thread_func, job->params, FALSE, job->error);
		break;
	    case OP_QUERY_INSTALLED:
		g_thread_create((GThreadFunc)disnix_query_installed_thread_func, job->params, FALSE, job->error);
		break;
	    case OP_QUERY_REQUISITES:
		g_thread_create((GThreadFunc)disnix_query_requisites_thread_func, job->params, FALSE, job->error);
		break;
	    case OP_COLLECT_GARBAGE:
		g_thread_create((GThreadFunc)disnix_collect_garbage_thread_func, job->params, FALSE, job->error);
		break;
	    case OP_ACTIVATE:
		g_thread_create((GThreadFunc)disnix_activate_thread_func, job->params, FALSE, job->error);
		break;
	    case OP_DEACTIVATE:
		g_thread_create((GThreadFunc)disnix_deactivate_thread_func, job->params, FALSE, job->error);
		break;
	}
	
	g_hash_table_remove(job_table, pid);
	delete_job(job);
    }
    
    return TRUE;
}
