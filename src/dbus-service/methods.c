#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <glib.h>
#include "disnix-instance-def.h"
#include "hash.h"
#define BUFFER_SIZE 1024

extern char *activation_modules_dir;

static gchar *generate_derivations_string(gchar **derivation, char *separator)
{
    int i;
    gchar *list = g_strdup("");
    
    while(*derivation != NULL)
    {
	gchar *old_list = list;
	list = g_strconcat(old_list, separator, *derivation);
	g_free(old_list);
	
	derivation++;
    }
    
    return list;
}

/* Import method */

typedef struct
{
    gchar **derivation;
    gchar *pid;
    DisnixObject *object;
}
DisnixImportParams;

static void disnix_import_thread_func(gpointer data)
{
    /* Declarations */
    gchar **derivation, *derivations_string, *cmd, *pid;
    FILE *fp;
    char line[BUFFER_SIZE];
    DisnixImportParams *params;
    
    /* Import variables */
    params = (DisnixImportParams*)data;
    derivation = params->derivation;
    pid = params->pid;

    /* Generate derivation strings */
    derivations_string = generate_derivation_strings(derivation, " ");
    
    /* Print log entry */
    g_print("Importing: %s\n", derivations_string);
    
    /* Execute command */
    cmd = g_strconcat("cat ", derivations_string, " | nix-store --import ", NULL);
    
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
    g_strfreev(derivation);
}

gboolean disnix_import(DisnixObject *object, gchar **derivation, gchar **pid, GError **error)
{
    /* Declarations */
    gchar *pidstring;
    gchar *derivations_string;
    DisnixImportParams *params;
    
    /* State object should not be NULL */
    g_assert(object != NULL);

    /* Generate derivations string */
    generate_derivations_string(derivation, ":");

    /* Generate process id */    
    pidstring = g_strconcat("import", derivations_string, NULL);
    object->pid = string_to_hash(pidstring);
    *pid = object->pid;
    g_free(pidstring);
    g_free(derivations_string);
    
    /* Create parameter struct */
    params = (DisnixImportParams*)g_malloc(sizeof(DisnixImportParams));
    params->derivation = derivation;
    params->pid = g_strdup(object->pid);
    params->object = object;
    
    /* Create thread */
    g_thread_create((GThreadFunc)disnix_import_thread_func, params, FALSE, error);
    
    return TRUE;
}


gboolean disnix_export(DisnixObject *object, gchar **derivation, gchar **pid, GError **error)
{
}

gboolean disnix_print_invalid(DisnixObject *object, gchar **derivation, gchar **pid, GError **error)
{
}

gboolean disnix_realise(DisnixObject *object, gchar **derivation, gchar **pid, GError **error)
{
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
    
    mkdir("/nix/var/nix/profiles/disnix", 0755);
    cmd = g_strconcat("nix-env -p /nix/var/nix/profiles/disnix/", profile, " --set ", derivation, NULL);

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
    
    /* Create thread */
    g_thread_create((GThreadFunc)disnix_set_thread_func, params, FALSE, error);
    
    return TRUE;
}

gboolean disnix_query_installed(DisnixObject *object, const gchar *profile, gchar **pid, GError **error)
{
}

gboolean disnix_collect_garbage(DisnixObject *object, const gboolean delete_old, gchar **pid, GError **error)
{
}

gboolean disnix_activate(DisnixObject *object, const gchar *derivation, gchar **arguments, gchar **pid, GError **error)
{
}

gboolean disnix_deactivate(DisnixObject *object, const gchar *derivation, gchar **arguments, gchar **pid, GError **error)
{
}

gboolean disnix_lock(DisnixObject *object, gchar **pid, GError **error)
{
}

gboolean disnix_unlock(DisnixObject *object, gchar **pid, GError **error)
{
}
