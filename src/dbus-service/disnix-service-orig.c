/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2009  Sander van der Burg
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

#include <dbus/dbus-glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "hash.h"
#define BUFFER_SIZE 512

typedef enum
{
    E_FINISH_SIGNAL,
    E_SUCCESS_SIGNAL,
    E_FAILURE_SIGNAL,
    E_LAST_SIGNAL
}
DisnixSignalNumber;

typedef struct
{
    GObject parent;
    gchar *pid;
}
DisnixObject;

typedef struct 
{
    GObjectClass parent;
    /* Signals created for this class */
    guint signals[E_LAST_SIGNAL];
}
DisnixObjectClass;

/* Forward declaration of the function that will return the GType of
   the Value implementation. Not used in this program. */
GType disnix_object_get_type (void);

/* Macro for the above. It is common to define macros using the
   naming convention (seen below) for all GType implementations,
   and that's why we're going to do that here as well. */
#define DISNIX_TYPE_OBJECT              (disnix_object_get_type ())
#define DISNIX_OBJECT(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), DISNIX_TYPE_OBJECT, DisnixObject))
#define DISNIX_OBJECT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), DISNIX_TYPE_OBJECT, DisnixObjectClass))
#define DISNIX_IS_OBJECT(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), DISNIX_TYPE_OBJECT))
#define DISNIX_IS_OBJECT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), DISNIX_TYPE_OBJECT))
#define DISNIX_OBJECT_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), DISNIX_TYPE_OBJECT, DisnixObjectClass))

G_DEFINE_TYPE(DisnixObject, disnix_object, G_TYPE_OBJECT)

/*
 * Since the stub generator will reference the functions from a call
 * table, the functions must be declared before the stub is included.
 */
gboolean disnix_install(DisnixObject *obj, const gchar *file, const gchar *args, const gboolean isAttr, gchar **pid, GError **error);

gboolean disnix_upgrade(DisnixObject *obj, const gchar *derivation, gchar **pid, GError **error);

gboolean disnix_uninstall(DisnixObject *obj, const gchar *derivation, gchar **pid, GError **error);

gboolean disnix_set(DisnixObject *obj, const gchar *profile, const gchar *derivation, gchar **pid, GError **error);

gboolean disnix_instantiate(DisnixObject *obj, const gchar *files, const gchar *attrPath, gchar **pid, GError **error);

gboolean disnix_realise(DisnixObject *obj, const gchar *derivation, gchar **pid, GError **error);

gboolean disnix_import(DisnixObject *obj, const gchar *path, gchar **pid, GError **error);

gboolean disnix_print_invalid_paths(DisnixObject *obj, const gchar *path, gchar **pid, GError **error);

gboolean disnix_collect_garbage(DisnixObject *obj, const gboolean delete_old, gchar **pid, GError **error);

gboolean disnix_activate(DisnixObject *obj, const gchar *path, const gchar *type, const gchar *args, gchar **pid, GError **error);

gboolean disnix_deactivate(DisnixObject *obj, const gchar *path, const gchar *type, const gchar *args, gchar **pid, GError **error);

#include "disnix-service.h"

char *activation_scripts;

/*
 * Object initializer
 * Set ret_value to 0
 */
static void
disnix_object_init (DisnixObject *obj)
{
    g_assert(obj != NULL);
    obj->pid = NULL;    
}

/*
 * Class initializer
 */
static void
disnix_object_class_init (DisnixObjectClass *klass)
{
    g_print (" : sample_object_class_init is called.\n");
    g_assert(klass != NULL);

    /* Create the signals */
    klass->signals[E_FINISH_SIGNAL] = g_signal_new ("finish", /* signal name */
                                                    G_OBJECT_CLASS_TYPE (klass), /* proxy object type */
						    G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED, /* signal flags */
						    0,
						    NULL, NULL,
						    g_cclosure_marshal_VOID__STRING,
						    G_TYPE_NONE, 
						    1, /* Number of parameter types to follow */
						    G_TYPE_STRING /* Parameter types */);
						    
    klass->signals[E_SUCCESS_SIGNAL] = g_signal_new ("success",
                                                    G_OBJECT_CLASS_TYPE (klass),
						    G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
						    0,
						    NULL, NULL,
						    g_cclosure_marshal_VOID__STRING,
						    G_TYPE_NONE,
						    2,
						    G_TYPE_STRING, G_TYPE_STRING);

    klass->signals[E_FAILURE_SIGNAL] = g_signal_new ("failure",
                                                    G_OBJECT_CLASS_TYPE (klass),
						    G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
						    0,
						    NULL, NULL,
						    g_cclosure_marshal_VOID__STRING,
						    G_TYPE_NONE,
						    1,
						    G_TYPE_STRING);
    
    g_print (" : Binding to Glib/D-Bus.\n");
    
    /* Bind this GType into the Glib/D-Bus wrappers */
    dbus_g_object_type_install_info (DISNIX_TYPE_OBJECT, &dbus_glib_disnix_object_info);
}

/* RPC methods */

/* Install function */

typedef struct
{
    gchar *file;
    gchar *args;
    gboolean isAttr;
    gchar *pid;
    DisnixObject *obj;
}
DisnixInstallParams;

static void disnix_install_thread_func(gpointer data)
{
    /* Declarations */
    gchar *cmd, *oldCmd, *file, *args, *pid;
    gboolean isAttr;
    FILE *fp;
    char line[BUFFER_SIZE];
    DisnixInstallParams *params;
    
    /* Import variables */
    params = (DisnixInstallParams*)data;
    file = params->file;
    args = params->args;
    isAttr = params->isAttr;
    pid = params->pid;
    
    /* Print log entry */
    if(file != NULL && strcmp(file, "") != 0)
	g_print("Using nix expression: %s\n", file);
    
    if(isAttr)
	g_print("Installing attribute path %s\n", args);
    else
	g_print("Installing %s\n", args);
    
    /* Generate command string */
    cmd = g_strdup("nix-env -i ");
    
    if(file != NULL && strcmp(file, "") != 0)
    {
	oldCmd = cmd;
	cmd = g_strconcat(oldCmd, "-f ", file, " ", NULL);
	g_free(oldCmd);
    }
    
    if(isAttr)
    {
	oldCmd = cmd;
	cmd = g_strconcat(oldCmd, "-A ", NULL);
	g_free(oldCmd);
    }
    
    oldCmd = cmd;
    cmd = g_strconcat(oldCmd, args, NULL);
    g_free(oldCmd);
    
    /* Execute command */
    fp = popen(cmd, "r");
    if(fp == NULL)
	disnix_emit_failure_signal(params->obj, pid); /* Something went wrong with forking the process */
    else
    {
	int status;
	
	while(fgets(line, sizeof(line), fp) != NULL)
	    puts(line);
	
	status = pclose(fp);
	
	if(status == -1 || WEXITSTATUS(status) != 0)
	    disnix_emit_failure_signal(params->obj, pid);
	else
	    disnix_emit_finish_signal(params->obj, pid);
    }
    
    /* Free variables */
    g_free(cmd);
    g_free(args);
    g_free(pid);
    g_free(params);
}

gboolean disnix_install(DisnixObject *obj, const gchar *file, const gchar *args, const gboolean isAttr, gchar **pid, GError **error)
{
    /* Declarations */
    gchar *pidstring;
    DisnixInstallParams *params;
    
    /* State object should not be NULL */
    g_assert(obj != NULL);

    /* Generate process id */
    pidstring = g_strconcat("install:", args, NULL);
    obj->pid = string_to_hash(pidstring);
    *pid = obj->pid;
    g_free(pidstring);

    /* Create parameter struct */
    params = (DisnixInstallParams*)g_malloc(sizeof(DisnixInstallParams));
    params->file = g_strdup(file);
    params->args = g_strdup(args);
    params->isAttr = isAttr;
    params->pid = g_strdup(obj->pid);
    params->obj = obj;
    
    /* Create thread */
    g_thread_create((GThreadFunc)disnix_install_thread_func, params, FALSE, error);
            
    return TRUE;
}

/* Upgrade function */

typedef struct
{
    gchar *derivation;
    gchar *pid;
    DisnixObject *obj;
}
DisnixUpgradeParams;

static void disnix_upgrade_thread_func(gpointer data)
{
    /* Declarations */
    gchar *cmd, *derivation, *pid;
    FILE *fp;
    char line[BUFFER_SIZE];
    DisnixUpgradeParams *params;
    
    /* Import variables */
    params = (DisnixUpgradeParams*)data;
    derivation = params->derivation;
    pid = params->pid;
    
    /* Print log entry */
    g_print("Upgrading %s\n", derivation);
    
    /* Execute command */
    cmd = g_strconcat("nix-env -u ", derivation, NULL);    
    
    fp = popen(cmd, "r");
    if(fp == NULL)
	disnix_emit_failure_signal(params->obj, pid); /* Something went wrong with forking the process */
    else
    {
	int status;
	
	while(fgets(line, sizeof(line), fp) != NULL)
	    puts(line);
	
	status = pclose(fp);
	
	if(status == -1 || WEXITSTATUS(status) != 0)
	    disnix_emit_failure_signal(params->obj, pid);
	else
	    disnix_emit_finish_signal(params->obj, pid);
    }
    
    /* Free variables */
    g_free(derivation);
    g_free(pid);
    g_free(params);
    g_free(cmd);
}

gboolean disnix_upgrade(DisnixObject *obj, const gchar *derivation, gchar **pid, GError **error)
{
    /* Declarations */
    gchar *pidstring;
    DisnixUpgradeParams *params;
    
    /* State object should not be NULL */
    g_assert(obj != NULL);
    
    /* Generate process id */
    pidstring = g_strconcat("upgrade:", derivation, NULL);
    obj->pid = string_to_hash(pidstring);
    *pid = obj->pid;
    g_free(pidstring);
    
    /* Create parameter struct */
    params = (DisnixUpgradeParams*)g_malloc(sizeof(DisnixUpgradeParams));
    params->derivation = g_strdup(derivation);
    params->pid = g_strdup(obj->pid);
    params->obj = obj;
    
    /* Create thread */
    g_thread_create((GThreadFunc)disnix_upgrade_thread_func, params, FALSE, error);
    
    return TRUE;
}

/* Uninstall function */

typedef struct
{
    gchar *derivation;
    gchar *pid;
    DisnixObject *obj;
}
DisnixUninstallParams;

static void disnix_uninstall_thread_func(gpointer data)
{
    /* Declarations */
    gchar *cmd, *derivation, *pid;
    FILE *fp;
    char line[BUFFER_SIZE];
    DisnixUninstallParams *params;
    
    /* Import variables */
    params = (DisnixUninstallParams*)data;
    derivation = params->derivation;
    pid = params->pid;
    
    /* Print log entry */
    g_print("Uninstalling %s\n", derivation);
    
    /* Execute command */
    cmd = g_strconcat("nix-env -e ", derivation, NULL);

    fp = popen(cmd, "r");
    if(fp == NULL)
	disnix_emit_failure_signal(params->obj, pid); /* Something went wrong with forking the process */
    else
    {
	int status;
	
	while(fgets(line, sizeof(line), fp) != NULL)
	    puts(line);
	
	status = pclose(fp);
	
	if(status == -1 || WEXITSTATUS(status) != 0)
	    disnix_emit_failure_signal(params->obj, pid);
	else
	    disnix_emit_finish_signal(params->obj, pid);
    }
    
    /* Free variables */
    g_free(derivation);
    g_free(pid);
    g_free(params);
    g_free(cmd);
}

gboolean disnix_uninstall(DisnixObject *obj, const gchar *derivation, gchar **pid, GError **error)
{
    /* Declarations */
    gchar *pidstring;
    FILE *fp;
    char line[BUFFER_SIZE];
    DisnixUninstallParams *params;
    
    /* State object should not be NULL */
    g_assert(obj != NULL);
    
    /* Generate process id */
    pidstring = g_strconcat("uninstall:", derivation, NULL);
    obj->pid = string_to_hash(pidstring);
    *pid = obj->pid;
    g_free(pidstring);
    
    /* Create parameter struct */
    params = (DisnixUninstallParams*)g_malloc(sizeof(DisnixUninstallParams));
    params->derivation = g_strdup(derivation);
    params->pid = g_strdup(obj->pid);
    params->obj = obj;
    
    /* Create thread */
    g_thread_create((GThreadFunc)disnix_uninstall_thread_func, params, FALSE, error);
    
    return TRUE;
}

/* Uninstall function */

typedef struct
{
    gchar *profile;
    gchar *derivation;    
    gchar *pid;
    DisnixObject *obj;
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
	disnix_emit_failure_signal(params->obj, pid); /* Something went wrong with forking the process */
    else
    {
	int status;
	
	while(fgets(line, sizeof(line), fp) != NULL)
	    puts(line);
	
	status = pclose(fp);
	
	if(status == -1 || WEXITSTATUS(status) != 0)
	    disnix_emit_failure_signal(params->obj, pid);
	else
	    disnix_emit_finish_signal(params->obj, pid);
    }
    
    /* Free variables */
    g_free(profile);
    g_free(derivation);
    g_free(pid);
    g_free(params);
    g_free(cmd);
}


gboolean disnix_set(DisnixObject *obj, const gchar *profile, const gchar *derivation, gchar **pid, GError **error)
{
    /* Declarations */
    gchar *pidstring;
    FILE *fp;
    char line[BUFFER_SIZE];
    DisnixSetParams *params;
    
    /* State object should not be NULL */
    g_assert(obj != NULL);
    
    /* Generate process id */
    pidstring = g_strconcat("set:", profile, ":", derivation, NULL);
    obj->pid = string_to_hash(pidstring);
    *pid = obj->pid;
    g_free(pidstring);
    
    /* Create parameter struct */
    params = (DisnixSetParams*)g_malloc(sizeof(DisnixSetParams));
    params->profile = g_strdup(profile);
    params->derivation = g_strdup(derivation);
    params->pid = g_strdup(obj->pid);
    params->obj = obj;
    
    /* Create thread */
    g_thread_create((GThreadFunc)disnix_set_thread_func, params, FALSE, error);
    
    return TRUE;
}

/* Instantiate function */

typedef struct
{
    gchar *files;
    gchar *attrPath;
    gchar *pid;
    DisnixObject *obj;
}
DisnixInstantiateParams;

static void disnix_instantiate_thread_func(gpointer data)
{
    /* Declarations */
    gchar *cmd, *oldCmd, *files, *attrPath, *pid, *missingPaths = NULL, *oldMissingPaths = NULL;;
    FILE *fp;
    char line[BUFFER_SIZE];
    DisnixInstantiateParams *params;
    
    /* Import variables */
    params = (DisnixInstantiateParams*)data;
    files = params->files;
    attrPath = params->attrPath;
    pid = params->pid;
    
    /* Print log entry */
    g_print("Executing instantiate: %s with attribute path: %s\n", files, attrPath);
    
    /* Generate command string */

    cmd = g_strdup("nix-instantiate");
    
    if(attrPath != NULL && strcmp(attrPath, "") != 0)
    {
	oldCmd = cmd;
	cmd = g_strconcat(oldCmd, " -A ", attrPath, NULL);
	g_free(oldCmd);
    }
    
    oldCmd = cmd;
    cmd = g_strconcat(oldCmd, " ", files, NULL);
    g_free(oldCmd);
    
    /* Execute command */
        
    fp = popen(cmd, "r");
    if(fp == NULL)
	disnix_emit_failure_signal(params->obj, pid); /* Something went wrong with forking the process */
    else
    {
	int status;
	
	/* Initialize missing paths */
	missingPaths = g_strdup("");	
    
	/* Read the output */
	while(fgets(line, sizeof(line), fp) != NULL)
	{
	    puts(line);
	    oldMissingPaths = missingPaths;	    	    
	    missingPaths = g_strconcat(oldMissingPaths, line, NULL);
	    g_free(oldMissingPaths);	    
	}
	
	status = pclose(fp);
	
	if(status == -1 || WEXITSTATUS(status) != 0)
	    disnix_emit_failure_signal(params->obj, pid);
	else
	    disnix_emit_success_signal(params->obj, pid, missingPaths);
    }
    
    /* Free variables */
    g_free(files);
    g_free(attrPath);
    g_free(pid);
    g_free(params);
    g_free(cmd);
}

gboolean disnix_instantiate(DisnixObject *obj, const gchar *files, const gchar *attrPath, gchar **pid, GError **error)
{
    /* Declarations */
    gchar *pidstring;
    DisnixInstantiateParams *params;

    /* State object should not be NULL */
    g_assert(obj != NULL);
    
    /* Generate process id */
    pidstring = g_strconcat("instantiate:", files, ":", attrPath, NULL);
    obj->pid = string_to_hash(pidstring);
    *pid = obj->pid;
    g_free(pidstring);

    /* Create parameter struct */
    params = (DisnixInstantiateParams*)g_malloc(sizeof(DisnixInstantiateParams));
    params->files = g_strdup(files);
    params->attrPath = g_strdup(attrPath);
    params->pid = g_strdup(obj->pid);
    params->obj = obj;

    /* Create thread */
    g_thread_create((GThreadFunc)disnix_instantiate_thread_func, params, FALSE, error);

    return TRUE;
}

typedef struct
{
    gchar *derivation;
    gchar *pid;
    DisnixObject *obj;
}
DisnixRealiseParams;

static void disnix_realise_thread_func(gpointer data)
{
    /* Declarations */
    gchar *cmd, *derivation, *path, *pid;
    FILE *fp;
    char line[BUFFER_SIZE];
    DisnixRealiseParams *params;
    
    /* Import variables */
    params = (DisnixRealiseParams*)data;
    derivation = params->derivation;
    pid = params->pid;
    
    /* Print log entry */
    g_print("Realising %s\n", derivation);
    
    /* Execute command */    
    cmd = g_strconcat("nix-store -r ", derivation, NULL);
    
    fp = popen(cmd, "r");
    if(fp == NULL)
	disnix_emit_failure_signal(params->obj, pid); /* Something went wrong with forking the process */
    else
    {
	int status;
	
	while(fgets(line, sizeof(line), fp) != NULL)
	    puts(line);
	
	status = pclose(fp);
	
	if(status == -1 || WEXITSTATUS(status) != 0)
	    disnix_emit_failure_signal(params->obj, pid);
	else
	{
	    /* Emit success signal */
	    path = line;
	    disnix_emit_success_signal(params->obj, pid, path);
	}
	    
    }
    
    /* Free variables */
    g_free(derivation);
    g_free(pid);
    g_free(params);
    g_free(cmd);
}

gboolean disnix_realise(DisnixObject *obj, const gchar *derivation, gchar **pid, GError **error)
{
    /* Declarations */
    gchar *pidstring;
    DisnixRealiseParams *params;
    
    /* State object should not be NULL */
    g_assert(obj != NULL);

    /* Generate process id */
    pidstring = g_strconcat("realise:", derivation, NULL);
    obj->pid = string_to_hash(pidstring);
    *pid = obj->pid;
    g_free(pidstring);

    /* Create parameter struct */
    params = (DisnixRealiseParams*)g_malloc(sizeof(DisnixRealiseParams));
    params->derivation = g_strdup(derivation);
    params->pid = g_strdup(obj->pid);
    params->obj = obj;
    
    /* Create thread */
    g_thread_create((GThreadFunc)disnix_realise_thread_func, params, FALSE, error);

    return TRUE;
}

/* Import function */

typedef struct
{
    gchar *path;
    gchar *pid;
    DisnixObject *obj;
}
DisnixImportParams;

static void disnix_import_thread_func(gpointer data)
{
    /* Declarations */
    gchar *path, *cmd, *pid;
    FILE *fp;
    char line[BUFFER_SIZE];
    DisnixImportParams *params;
    
    /* Import variables */
    params = (DisnixImportParams*)data;
    path = params->path;
    pid = params->pid;
    
    /* Print log entry */
    g_print("Importing: %s\n", path);
    
    /* Execute command */
    cmd = g_strconcat("cat ", path, " | nix-store --import ", NULL);
    
    fp = popen(cmd, "r");
    if(fp == NULL)
	disnix_emit_failure_signal(params->obj, pid); /* Something went wrong with forking the process */
    else
    {
	int status;
	
	while(fgets(line, sizeof(line), fp) != NULL)
	    puts(line);
	
	status = pclose(fp);
	
	if(status == -1 || WEXITSTATUS(status) != 0)
	    disnix_emit_failure_signal(params->obj, pid);
	else
	    disnix_emit_finish_signal(params->obj, pid);
    }
    
    /* Free variables */
    g_free(path);
    g_free(pid);
    g_free(params);
    g_free(cmd);
}

gboolean disnix_import(DisnixObject *obj, const gchar *path, gchar **pid, GError **error)
{
    /* Declarations */
    gchar *pidstring;
    DisnixImportParams *params;
    
    /* State object should not be NULL */
    g_assert(obj != NULL);

    /* Generate process id */
    pidstring = g_strconcat("import:", path, NULL);
    obj->pid = string_to_hash(pidstring);
    *pid = obj->pid;
    g_free(pidstring);
    
    /* Create parameter struct */
    params = (DisnixImportParams*)g_malloc(sizeof(DisnixImportParams));
    params->path = g_strdup(path);
    params->pid = g_strdup(obj->pid);
    params->obj = obj;
    
    /* Create thread */
    g_thread_create((GThreadFunc)disnix_import_thread_func, params, FALSE, error);

    return TRUE;
}

/* Print invalid paths function */

typedef struct
{
    gchar *path;
    gchar *pid;
    DisnixObject *obj;
}
DisnixPrintInvalidPathsParams;

static void disnix_print_invalid_paths_thread_func(gpointer data)
{
    /* Declarations */
    gchar *path, *cmd, *pid, *missingPaths = NULL, *oldMissingPaths = NULL;
    FILE *fp;
    char line[BUFFER_SIZE];
    DisnixPrintInvalidPathsParams *params;

    /* Import variables */
    params = (DisnixPrintInvalidPathsParams*)data;
    path = params->path;
    pid = params->pid;
    
    /* Print log entry */
    g_print("Print invalid paths: %s\n", path);
    
    /* Execute command */
    cmd = g_strconcat("nix-store --check-validity --print-invalid ", path, NULL);
    
    fp = popen(cmd, "r");
    if(fp == NULL)
	disnix_emit_failure_signal(params->obj, pid); /* Something went wrong with forking the process */
    else
    {
	int status;
	
	/* Initialize missing paths */
	missingPaths = g_strdup("");	
	
	/* Read the output */
	while(fgets(line, sizeof(line), fp) != NULL)
	{
	    puts(line);
	    oldMissingPaths = missingPaths;	    	    
	    missingPaths = g_strconcat(oldMissingPaths, line, NULL);
	    g_free(oldMissingPaths);	    
	}
	
	status = pclose(fp);
	
	if(status == -1 || WEXITSTATUS(status) != 0)
	    disnix_emit_failure_signal(params->obj, pid);
	else
	    disnix_emit_success_signal(params->obj, pid, missingPaths);
    }
    
    /* Free variables */
    g_free(path);
    g_free(pid);
    g_free(params);
    g_free(cmd);    
}

gboolean disnix_print_invalid_paths(DisnixObject *obj, const gchar *path, gchar **pid, GError **error)
{
    /* Declarations */
    gchar *pidstring;
    DisnixPrintInvalidPathsParams *params;

    /* State object should not be NULL */
    g_assert(obj != NULL);

    /* Generate process id */
    pidstring = g_strconcat("print_invalid_paths:", path, NULL);
    obj->pid = string_to_hash(pidstring);
    *pid = obj->pid;
    g_free(pidstring);
    
    /* Create parameter struct */
    params = (DisnixPrintInvalidPathsParams*)g_malloc(sizeof(DisnixPrintInvalidPathsParams));
    params->path = g_strdup(path);
    params->pid = g_strdup(obj->pid);
    params->obj = obj;

    /* Create thread */
    g_thread_create((GThreadFunc)disnix_print_invalid_paths_thread_func, params, FALSE, error);    
    
    return TRUE;
}

/* Garbage collect function */

typedef struct
{
    gboolean delete_old;
    gchar *pid;
    DisnixObject *obj;
}
DisnixGarbageCollectParams;

static void disnix_garbage_collect_thread_func(gpointer data)
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
	disnix_emit_failure_signal(params->obj, pid); /* Something went wrong with forking the process */
    else
    {
	int status;
	
	/* Read the output */
	while(fgets(line, sizeof(line), fp) != NULL)
	    puts(line);
	
	status = pclose(fp);
	
	if(status == -1 || WEXITSTATUS(status) != 0)
	    disnix_emit_failure_signal(params->obj, pid);
	else
	    disnix_emit_finish_signal(params->obj, pid);
    }
    
    /* Free variables */
    g_free(pid);
    g_free(params);
    g_free(cmd);    
}

gboolean disnix_collect_garbage(DisnixObject *obj, const gboolean delete_old, gchar **pid, GError **error)
{
    /* Declarations */
    gchar *pidstring;
    DisnixGarbageCollectParams *params;

    /* State object should not be NULL */
    g_assert(obj != NULL);

    /* Generate process id */
    if(delete_old)
	pidstring = g_strdup("collect_garbage:delete_old");
    else
	pidstring = g_strdup("collect_garbage");
	
    obj->pid = string_to_hash(pidstring);
    *pid = obj->pid;
    g_free(pidstring);
    
    /* Create parameter struct */
    params = (DisnixGarbageCollectParams*)g_malloc(sizeof(DisnixGarbageCollectParams));
    params->delete_old = delete_old;
    params->pid = g_strdup(obj->pid);
    params->obj = obj;

    /* Create thread */
    g_thread_create((GThreadFunc)disnix_garbage_collect_thread_func, params, FALSE, error);    
    
    return TRUE;
}

/* Activate function */

typedef struct
{
    gchar *path;
    gchar *type;
    gchar *args;
    gchar *pid;
    DisnixObject *obj;
}
DisnixActivateParams;

static void disnix_activate_thread_func(gpointer data)
{
    /* Declarations */
    gchar *path, *type, *args, *cmd, *pid;
    FILE *fp;
    char line[BUFFER_SIZE];
    DisnixActivateParams *params;

    /* Import variables */
    params = (DisnixActivateParams*)data;
    path = params->path;
    type = params->type;
    args = params->args;
    pid = params->pid;
    
    /* Print log entry */
    g_print("Activate: %s\n", path);
    
    /* Execute command */

    cmd = g_strconcat("env ", args, " ", activation_scripts, "/", type, " activate ", path, NULL);
    
    fp = popen(cmd, "r");
    if(fp == NULL)
        disnix_emit_failure_signal(params->obj, pid); /* Something went wrong with forking the process */
    else
    {
	int status;
	
        /* Read the output */
        while(fgets(line, sizeof(line), fp) != NULL)
    	    puts(line);
	
	status = pclose(fp);
	
	if(status == -1 || WEXITSTATUS(status) != 0)
	    disnix_emit_failure_signal(params->obj, pid);
	else
	    disnix_emit_finish_signal(params->obj, pid);
    }

    /* Free variables */
    g_free(path);
    g_free(type);
    g_free(args);
    g_free(pid);
    g_free(params);
    g_free(cmd);    
}

gboolean disnix_activate(DisnixObject *obj, const gchar *path, const gchar *type, const gchar *args, gchar **pid, GError **error)
{
    /* Declarations */
    gchar *pidstring;
    DisnixActivateParams *params;

    /* State object should not be NULL */
    g_assert(obj != NULL);

    /* Generate process id */
    pidstring = g_strconcat("activate:", path, ":", type, ":", args, NULL);
    obj->pid = string_to_hash(pidstring);
    *pid = obj->pid;
    g_free(pidstring);
    
    /* Create parameter struct */
    params = (DisnixActivateParams*)g_malloc(sizeof(DisnixActivateParams));
    params->path = g_strdup(path);
    params->type = g_strdup(type);
    params->args = g_strdup(args);
    params->pid = g_strdup(obj->pid);
    params->obj = obj;

    /* Create thread */
    g_thread_create((GThreadFunc)disnix_activate_thread_func, params, FALSE, error);    
    
    return TRUE;    
}

/* Deactivate function */

typedef struct
{
    gchar *path;
    gchar *type;
    gchar *args;
    gchar *pid;
    DisnixObject *obj;
}
DisnixDeactivateParams;

static void disnix_deactivate_thread_func(gpointer data)
{
    /* Declarations */
    gchar *path, *type, *args, *cmd, *pid;
    FILE *fp;
    char line[BUFFER_SIZE];
    DisnixDeactivateParams *params;

    /* Import variables */
    params = (DisnixDeactivateParams*)data;
    path = params->path;
    type = params->type;
    args = params->args;
    pid = params->pid;
    
    /* Print log entry */
    g_print("Deactivate: %s\n", path);
    
    /* Execute command */

    cmd = g_strconcat("env ", args, " ", activation_scripts, "/", type, " deactivate ", path, NULL);
    
    fp = popen(cmd, "r");
    if(fp == NULL)
        disnix_emit_failure_signal(params->obj, pid); /* Something went wrong with forking the process */
    else
    {
	int status;
	
        /* Read the output */
        while(fgets(line, sizeof(line), fp) != NULL)
  	    puts(line);
	
	status = pclose(fp);
	
	if(status == -1 || WEXITSTATUS(status) != 0)
	    disnix_emit_failure_signal(params->obj, pid);
	else
	    disnix_emit_finish_signal(params->obj, pid);
    }

    /* Free variables */
    g_free(path);
    g_free(type);
    g_free(args);
    g_free(pid);
    g_free(params);
    g_free(cmd);
}

gboolean disnix_deactivate(DisnixObject *obj, const gchar *path, const gchar *type, const gchar *args, gchar **pid, GError **error)
{
    /* Declarations */
    gchar *pidstring;
    DisnixDeactivateParams *params;

    /* State object should not be NULL */
    g_assert(obj != NULL);

    /* Generate process id */
    pidstring = g_strconcat("deactivate:", path, ":", type, ":", args, NULL);
    obj->pid = string_to_hash(pidstring);
    *pid = obj->pid;
    g_free(pidstring);
    
    /* Create parameter struct */
    params = (DisnixDeactivateParams*)g_malloc(sizeof(DisnixDeactivateParams));
    params->path = g_strdup(path);
    params->type = g_strdup(type);
    params->pid = g_strdup(obj->pid);
    params->obj = obj;

    /* Create thread */
    g_thread_create((GThreadFunc)disnix_deactivate_thread_func, params, FALSE, error);    
    
    return TRUE;    
}

/* Signal emit functions */

gboolean disnix_emit_finish_signal(DisnixObject *obj, gchar *pid)
{
    DisnixObjectClass *klass = DISNIX_OBJECT_GET_CLASS(obj);
    g_signal_emit(obj, klass->signals[E_FINISH_SIGNAL], 0, pid);
    return TRUE;
}

gboolean disnix_emit_success_signal(DisnixObject *obj, gchar *pid, gchar *path)
{
    DisnixObjectClass *klass = DISNIX_OBJECT_GET_CLASS(obj);
    g_signal_emit(obj, klass->signals[E_SUCCESS_SIGNAL], 0, pid, path);
    return TRUE;
}

gboolean disnix_emit_failure_signal(DisnixObject *obj, gchar *pid)
{
    DisnixObjectClass *klass = DISNIX_OBJECT_GET_CLASS(obj);
    g_signal_emit(obj, klass->signals[E_FAILURE_SIGNAL], 0, pid);
    return TRUE;
}

int main(int argc, char **argv)
{
    /* The GObject representing a D-Bus connection. */
    DBusGConnection *bus;
    /* Proxy object representing the D-Bus service object. */
    DBusGProxy *bus_proxy;
    /* Will hold one instance of DisnixObject that will serve all the requsts. */
    DisnixObject *obj;
    /* GMainLoop object */
    GMainLoop *mainloop;
    guint result;
    GError *error = NULL;
    
    /* Initialize the GType/GObject system. */
    g_type_init ();

    /* Initialize thread system */
    g_thread_init (NULL);
    
    /* Set the activation scripts directory */
    activation_scripts = getenv("ACTIVATION_SCRIPTS");
    
    /* Create a main loop that will dispatch callbacks. */
    mainloop = g_main_loop_new (NULL, FALSE);
    if (mainloop == NULL) {
      /* Print error and terminate. */
      g_print (" : Couldn't create GMainLoop.\n");
      return 1;
    }

    /*Connect to the session bus*/
    /*g_print (" : Connecting to Session D-Bus.\n");
    bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);*/
    
    g_print (" : Connecting to System D-Bus.\n");
    bus = dbus_g_bus_get (DBUS_BUS_SYSTEM, &error);
    
    if (bus == NULL) {
      g_print (" : Couldn't connect to session bus, %s\n", error->message);
      return 1;
    }

    /* In order to register a well-known name, we need to use the
     "RequestMethod" of the /org/freedesktop/DBus interface.*/

    bus_proxy = dbus_g_proxy_new_for_name (bus, "org.freedesktop.DBus",
					 "/org/freedesktop/DBus",
					 "org.freedesktop.DBus");
    /* Attempt to register the well-known name. */
    if (!dbus_g_proxy_call (bus_proxy, "RequestName", &error,
			    G_TYPE_STRING, "org.nixos.disnix.Disnix",
			    G_TYPE_UINT, 0,
			    G_TYPE_INVALID,
			    G_TYPE_UINT, &result,
			    G_TYPE_INVALID)) {
        /* Print error and terminate. */
        g_print (" : D-Bus.RequestName RPC failed %s\n", error->message);
        return 1;
    }

    g_print (" : RequestName returned %d.\n", result);
    /*Check the result*/
    if(result != 1) {
        /* Print error and terminate. */
        g_print (" : Failed to get the primary well-known name.\n");
        return 1;
    }
    
    /*Create one instance*/
    obj = g_object_new (DISNIX_TYPE_OBJECT, NULL);
    if(!obj) {
        /* Print error and terminate. */
        g_print (" : Failed to create one instance.\n");
        return 1;
    }
    /*Register the instance to D-Bus*/
    dbus_g_connection_register_g_object (bus, "/org/nixos/disnix/Disnix", G_OBJECT (obj));

    g_print ("Disnix service running\n");

    
    /* Run the program */
    g_main_loop_run(mainloop);

    return 1;
}
