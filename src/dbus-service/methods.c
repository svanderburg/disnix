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

#include "methods.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <glib.h>
#include "logging.h"
#include "profilemanifest.h"
#include "jobmanagement.h"
#include "package-management.h"
#include "state-management.h"

extern char *tmpdir, *logdir;

static void evaluate_boolean_process(pid_t pid, OrgNixosDisnixDisnix *object, gint jid)
{
    ProcReact_Status status;
    int result = procreact_wait_for_boolean(pid, &status);
    
    if(status == PROCREACT_STATUS_OK && result)
        org_nixos_disnix_disnix_emit_finish(object, jid);
    else
        org_nixos_disnix_disnix_emit_failure(object, jid);
}

static void evaluate_strv_process(gchar **result, OrgNixosDisnixDisnix *object, gint jid)
{
    if(result == NULL)
        org_nixos_disnix_disnix_emit_failure(object, jid);
    else
    {
        org_nixos_disnix_disnix_emit_success(object, jid, (const gchar**)result);
        g_strfreev(result);
    }
}

/* Get job id method */

gboolean on_handle_get_job_id(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation)
{
    int job_counter = assign_pid();
    g_printerr("Assigned job id: %d\n", job_counter);
    org_nixos_disnix_disnix_complete_get_job_id(object, invocation, job_counter);
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
        int closure_fd;
        
        /* Print log entry */
        dprintf(log_fd, "Importing: %s\n", params->arg_closure);
        
        /* Execute command */
        closure_fd = open(params->arg_closure, O_RDONLY);
        evaluate_boolean_process(pkgmgmt_import_closure(closure_fd, log_fd, log_fd), params->object, params->arg_pid);

        close(closure_fd);
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
        /* Print log entry */
        dprintf(log_fd, "Exporting: ");
        print_paths(log_fd, params->arg_derivation);
        dprintf(log_fd, "\n");
    
        /* Execute command */
        evaluate_strv_process(pkgmgmt_export_closure(tmpdir, params->arg_derivation, log_fd), params->object, params->arg_pid);
        
        close(log_fd);
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
        ProcReact_Future future;
        ProcReact_Status status;
        gchar **result;
        
        /* Print log entry */
        dprintf(log_fd, "Print invalid: ");
        print_paths(log_fd, params->arg_derivation);
        dprintf(log_fd, "\n");
        
        /* Execute command */
        
        future = pkgmgmt_print_invalid_packages(params->arg_derivation, log_fd);
        result = procreact_future_get(&future, &status);
        
        evaluate_strv_process(result, params->object, params->arg_pid);
        
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
        ProcReact_Future future;
        ProcReact_Status status;
        gchar **result;
        
        /* Print log entry */
        dprintf(log_fd, "Realising: ");
        print_paths(log_fd, params->arg_derivation);
        dprintf(log_fd, "\n");
        
        /* Execute command */
        
        future = pkgmgmt_realise(params->arg_derivation, log_fd);
        result = procreact_future_get(&future, &status);
        
        evaluate_strv_process(result, params->object, params->arg_pid);
        
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
        /* Print log entry */
        dprintf(log_fd, "Set profile: %s with derivation: %s\n", params->arg_profile, params->arg_derivation);
    
        /* Execute command */
        
        evaluate_boolean_process(pkgmgmt_set_profile(params->arg_profile, params->arg_derivation, log_fd, log_fd), params->object, params->arg_pid);
        
        close(log_fd);
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
        GPtrArray *profile_manifest_array;
    
        /* Print log entry */
        dprintf(log_fd, "Query installed derivations from profile: %s\n", params->arg_profile);
    
        /* Execute command */
        
        profile_manifest_array = create_profile_manifest_array(params->arg_profile);
    
        if(profile_manifest_array == NULL)
            org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
        else
        {
            gchar **derivations = query_derivations(profile_manifest_array);
            org_nixos_disnix_disnix_emit_success(params->object, params->arg_pid, (const gchar**)derivations);
            
            g_strfreev(derivations);
            delete_profile_manifest_array(profile_manifest_array);
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
        ProcReact_Future future;
        ProcReact_Status status;
        gchar **result;
        
        /* Print log entry */
        dprintf(log_fd, "Query requisites from derivations: ");
        print_paths(log_fd, params->arg_derivation);
        dprintf(log_fd, "\n");
    
        /* Execute command */
        
        future = pkgmgmt_query_requisites(params->arg_derivation, log_fd);
        result = procreact_future_get(&future, &status);
        
        evaluate_strv_process(result, params->object, params->arg_pid);
        
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
        /* Print log entry */
        if(params->arg_delete_old)
            dprintf(log_fd, "Garbage collect and remove old derivations\n");
        else
            dprintf(log_fd, "Garbage collect\n");
    
        /* Execute command */
        evaluate_boolean_process(pkgmgmt_collect_garbage(params->arg_delete_old, log_fd, log_fd), params->object, params->arg_pid);
        
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
    gchar *arg_container;
    gchar *arg_type;
    gchar **arg_arguments;
}
ActivityParams;

static ActivityParams *create_activity_params(OrgNixosDisnixDisnix *object, gchar *activity, gint arg_pid, const gchar *arg_derivation, const gchar *arg_container, const gchar *arg_type, const gchar *const *arg_arguments)
{
    ActivityParams *params = (ActivityParams*)g_malloc(sizeof(ActivityParams));
    
    params->object = object;
    params->activity = g_strdup(activity);
    params->arg_pid = arg_pid;
    params->arg_derivation = g_strdup(arg_derivation);
    params->arg_container = g_strdup(arg_container);
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
        /* Print log entry */
        dprintf(log_fd, "%s: %s of type: %s in container: %s with arguments: ", params->activity, params->arg_derivation, params->arg_type, params->arg_container);
        print_paths(log_fd, params->arg_arguments);
        dprintf(log_fd, "\n");
    
        /* Execute command */
        evaluate_boolean_process(statemgmt_run_dysnomia_activity(params->arg_type, params->activity, params->arg_derivation, params->arg_container, params->arg_arguments, log_fd, log_fd), params->object, params->arg_pid);
    
        close(log_fd);
    }
    
    /* Cleanup */
    g_free(params->activity);
    g_free(params->arg_derivation);
    g_free(params->arg_container);
    g_free(params->arg_type);
    g_strfreev(params->arg_arguments);
    g_free(params);
    return NULL;
}

/* Activate method */

gboolean on_handle_activate(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_derivation, const gchar *arg_container, const gchar *arg_type, const gchar *const *arg_arguments)
{
    ActivityParams *params = create_activity_params(object, "activate", arg_pid, arg_derivation, arg_container, arg_type, arg_arguments);
    GThread *thread = g_thread_new("activate", disnix_dysnomia_activity_thread_func, params);
    org_nixos_disnix_disnix_complete_activate(object, invocation);
    g_thread_unref(thread);
    return TRUE;
}

/* Deactivate method */

gboolean on_handle_deactivate(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_derivation, const gchar *arg_container, const gchar *arg_type, const gchar *const *arg_arguments)
{
    ActivityParams *params = create_activity_params(object, "deactivate", arg_pid, arg_derivation, arg_container, arg_type, arg_arguments);
    GThread *thread = g_thread_new("deactivate", disnix_dysnomia_activity_thread_func, params);
    org_nixos_disnix_disnix_complete_deactivate(object, invocation);
    g_thread_unref(thread);
    return TRUE;
}

/* Lock method */

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
        GPtrArray *profile_manifest_array;
        
        /* Print log entry */
        dprintf(log_fd, "Acquiring lock on profile: %s\n", params->arg_profile);
        
        /* Lock the disnix instance */
        profile_manifest_array = create_profile_manifest_array(params->arg_profile);
        
        if(profile_manifest_array == NULL)
        {
            dprintf(log_fd, "Corrupt profile manifest: a service or type is missing!\n");
            org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
        }
        else
        {
            if(acquire_locks(log_fd, profile_manifest_array))
                org_nixos_disnix_disnix_emit_finish(params->object, params->arg_pid);
            else
                org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
            
            /* Cleanup */
            delete_profile_manifest_array(profile_manifest_array);
        }
    
        close(log_fd);
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
        GPtrArray *profile_manifest_array;
        
        /* Print log entry */
        dprintf(log_fd, "Releasing lock on profile: %s\n", params->arg_profile);

        /* Unlock the Disnix instance */
        profile_manifest_array = create_profile_manifest_array(params->arg_profile);
        if(release_locks(log_fd, profile_manifest_array))
            org_nixos_disnix_disnix_emit_finish(params->object, params->arg_pid);
        else
            org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
        
        /* Cleanup */
        delete_profile_manifest_array(profile_manifest_array);
        close(log_fd);
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

gboolean on_handle_snapshot(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_derivation, const gchar *arg_container, const gchar *arg_type, const gchar *const *arg_arguments)
{
    ActivityParams *params = create_activity_params(object, "snapshot", arg_pid, arg_derivation, arg_container, arg_type, arg_arguments);
    GThread *thread = g_thread_new("snapshot", disnix_dysnomia_activity_thread_func, params);
    org_nixos_disnix_disnix_complete_snapshot(object, invocation);
    g_thread_unref(thread);
    return TRUE;
}

/* Restore method */

gboolean on_handle_restore(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_derivation, const gchar *arg_container, const gchar *arg_type, const gchar *const *arg_arguments)
{
    ActivityParams *params = create_activity_params(object, "restore", arg_pid, arg_derivation, arg_container, arg_type, arg_arguments);
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
        ProcReact_Future future;
        ProcReact_Status status;
        gchar **result;
        
        /* Print log entry */
        dprintf(log_fd, "Query all snapshots from container: %s and component: %s\n", params->arg_container, params->arg_component);
    
        /* Execute command */
        future = statemgmt_query_all_snapshots(params->arg_container, params->arg_component, log_fd);
        result = procreact_future_get(&future, &status);
        
        evaluate_strv_process(result, params->object, params->arg_pid);
        
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
        ProcReact_Future future;
        ProcReact_Status status;
        gchar **result;
        
        /* Print log entry */
        dprintf(log_fd, "Query latest snapshot from container: %s and component: %s\n", params->arg_container, params->arg_component);
    
        /* Execute command */
        future = statemgmt_query_latest_snapshot(params->arg_container, params->arg_component, log_fd);
        result = procreact_future_get(&future, &status);
        
        evaluate_strv_process(result, params->object, params->arg_pid);
        
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
        ProcReact_Future future;
        ProcReact_Status status;
        gchar **result;
        
        /* Print log entry */
        dprintf(log_fd, "Print missing snapshots: ");
        print_paths(log_fd, params->arg_component);
        dprintf(log_fd, "\n");
        
        /* Execute command */
        future = statemgmt_print_missing_snapshots(params->arg_component, log_fd);
        result = procreact_future_get(&future, &status);
        
        evaluate_strv_process(result, params->object, params->arg_pid);
        
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
        evaluate_boolean_process(statemgmt_import_snapshots(params->arg_container, params->arg_component, params->arg_snapshots, log_fd, log_fd), params->object, params->arg_pid);

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
        ProcReact_Future future;
        ProcReact_Status status;
        gchar **result;
        
        /* Print log entry */
        dprintf(log_fd, "Resolve snapshots: ");
        print_paths(log_fd, params->arg_snapshots);
        dprintf(log_fd, "\n");
        
        /* Execute command */
        future = statemgmt_resolve_snapshots(params->arg_snapshots, log_fd);
        result = procreact_future_get(&future, &status);

        evaluate_strv_process(result, params->object, params->arg_pid);
        
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
    gchar *arg_container;
    gchar *arg_component;
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
        char keepStr[15];
        
        /* Convert keep value to string */
        sprintf(keepStr, "%d", params->arg_keep);
        
        /* Print log entry */
        dprintf(log_fd, "Clean old snapshots");
        
        if(g_strcmp0(params->arg_container, "") != 0)
            dprintf(log_fd, " for container: %s", params->arg_container);
        
        if(g_strcmp0(params->arg_component, "") != 0)
            dprintf(log_fd, " for component: %s", params->arg_component);
        
        dprintf(log_fd, " num of generations to keep: %s!\n", keepStr);
        
        /* Execute command */
        evaluate_boolean_process(statemgmt_clean_snapshots(keepStr, params->arg_container, params->arg_component, log_fd, log_fd), params->object, params->arg_pid);

        close(log_fd);
    }
    
    /* Cleanup */
    g_free(params->arg_container);
    g_free(params->arg_component);
    g_free(params);
    return NULL;
}

gboolean on_handle_clean_snapshots(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, gint arg_keep, const gchar *arg_container, const char *arg_component)
{
    GThread *thread;
    
    CleanSnapshotsParams *params = (CleanSnapshotsParams*)g_malloc(sizeof(CleanSnapshotsParams));
    params->object = object;
    params->arg_pid = arg_pid;
    params->arg_keep = arg_keep;
    params->arg_container = g_strdup(arg_container);
    params->arg_component = g_strdup(arg_component);
    
    thread = g_thread_new("clean-snapshots", disnix_clean_snapshots_thread_func, params);
    org_nixos_disnix_disnix_complete_clean_snapshots(object, invocation);
    g_thread_unref(thread);
    return TRUE;
}

/* Delete state operation */

gboolean on_handle_delete_state(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_derivation, const gchar *arg_container, const gchar *arg_type, const gchar *const *arg_arguments)
{
    ActivityParams *params = create_activity_params(object, "collect-garbage", arg_pid, arg_derivation, arg_container, arg_type, arg_arguments);
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

/* Capture config operation */

typedef struct
{
    OrgNixosDisnixDisnix *object;
    gint arg_pid;
}
CaptureConfigParams;

static gpointer disnix_capture_config_thread_func(gpointer data)
{
    CaptureConfigParams *params = (CaptureConfigParams*)data;
    int log_fd = open_log_file(params->arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(params->object, params->arg_pid);
    }
    else
    {
        evaluate_strv_process(statemgmt_capture_config(tmpdir, log_fd), params->object, params->arg_pid);
        
        close(log_fd);
    }
    
    /* Cleanup */
    g_free(params);
    return NULL;

}

gboolean on_handle_capture_config(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid)
{
    GThread *thread;
    
    CaptureConfigParams *params = (CaptureConfigParams*)g_malloc(sizeof(CaptureConfigParams));
    params->object = object;
    params->arg_pid = arg_pid;

    thread = g_thread_new("capture-config", disnix_capture_config_thread_func, params);
    org_nixos_disnix_disnix_complete_capture_config(object, invocation);
    g_thread_unref(thread);
    return TRUE;
}
