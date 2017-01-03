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

typedef struct
{
    OrgNixosDisnixDisnix *object;
    gint jid;
    int log_fd;
    pid_t pid;
}
BooleanThreadData;

static gpointer evaluate_boolean_process_thread_func(gpointer data)
{
    ProcReact_Status status;
    BooleanThreadData *boolean_data = (BooleanThreadData*)data;
    int result = procreact_wait_for_boolean(boolean_data->pid, &status);
    
    if(status == PROCREACT_STATUS_OK && result)
        org_nixos_disnix_disnix_emit_finish(boolean_data->object, boolean_data->jid);
    else
        org_nixos_disnix_disnix_emit_failure(boolean_data->object, boolean_data->jid);
    
    /* Cleanup */
    g_free(boolean_data);
    close(boolean_data->log_fd);
    
    return NULL;
}

static void evaluate_boolean_process(pid_t pid, OrgNixosDisnixDisnix *object, gint jid, int log_fd)
{
    GThread *thread;
    BooleanThreadData *data = (BooleanThreadData*)g_malloc(sizeof(BooleanThreadData));
    
    data->object = object;
    data->jid = jid;
    data->log_fd = log_fd;
    data->pid = pid;
    
    thread = g_thread_new("evaluate-boolean", evaluate_boolean_process_thread_func, data);
    g_thread_unref(thread);
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

typedef struct
{
    OrgNixosDisnixDisnix *object;
    gint jid;
    int log_fd;
    ProcReact_Future future;
}
StrvThreadData;

static gpointer evaluate_strv_process2_thread_func(gpointer data)
{
    StrvThreadData *strv_data = (StrvThreadData*)data;
    ProcReact_Status status;
    char **result = procreact_future_get(&strv_data->future, &status);
    
    if(status != PROCREACT_STATUS_OK || result == NULL)
        org_nixos_disnix_disnix_emit_failure(strv_data->object, strv_data->jid);
    else
    {
        org_nixos_disnix_disnix_emit_success(strv_data->object, strv_data->jid, (const gchar**)result);
        procreact_free_string_array(result);
    }
    
    /* Cleanup */
    g_free(strv_data);
    close(strv_data->log_fd);
    
    return NULL;
}

static void evaluate_strv_process2(ProcReact_Future future, OrgNixosDisnixDisnix *object, gint jid, int log_fd)
{
    GThread *thread;
    StrvThreadData *data = (StrvThreadData*)g_malloc(sizeof(StrvThreadData));
    
    data->object = object;
    data->jid = jid;
    data->log_fd = log_fd;
    data->future = future;
    
    thread = g_thread_new("evaluate-strv", evaluate_strv_process2_thread_func, data);
    g_thread_unref(thread);
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

gboolean on_handle_import(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_closure)
{
    int log_fd = open_log_file(arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(object, arg_pid);
    }
    else
    {
        /* Print log entry */
        dprintf(log_fd, "Importing: %s\n", arg_closure);
        
        /* Execute command */
        evaluate_boolean_process(pkgmgmt_import_closure(arg_closure, log_fd, log_fd), object, arg_pid, log_fd);
    }
    
    org_nixos_disnix_disnix_complete_import(object, invocation);
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

gboolean on_handle_print_invalid(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *const *arg_derivation)
{
    int log_fd = open_log_file(arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile of pid: %d!\n", arg_pid);
        org_nixos_disnix_disnix_emit_failure(object, arg_pid);
    }
    else
    {
        /* Print log entry */
        dprintf(log_fd, "Print invalid: ");
        print_paths(log_fd, (gchar**)arg_derivation);
        dprintf(log_fd, "\n");
        
        /* Execute command */
        evaluate_strv_process2(pkgmgmt_print_invalid_packages((gchar**)arg_derivation, log_fd), object, arg_pid, log_fd);
    }
    
    org_nixos_disnix_disnix_complete_print_invalid(object, invocation);
    return TRUE;
}

/* Realise method */

gboolean on_handle_realise(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *const *arg_derivation)
{
    int log_fd = open_log_file(arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(object, arg_pid);
    }
    else
    {
        /* Print log entry */
        dprintf(log_fd, "Realising: ");
        print_paths(log_fd, (gchar**)arg_derivation);
        dprintf(log_fd, "\n");
        
        /* Execute command and wait and asychronously propagate its end result */
        evaluate_strv_process2(pkgmgmt_realise((gchar**)arg_derivation, log_fd), object, arg_pid, log_fd);
    }
    
    org_nixos_disnix_disnix_complete_realise(object, invocation);
    return TRUE;
}

/* Set method */

gboolean on_handle_set(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_profile, const gchar *arg_derivation)
{
    int log_fd = open_log_file(arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(object, arg_pid);
    }
    else
    {
        /* Print log entry */
        dprintf(log_fd, "Set profile: %s with derivation: %s\n", arg_profile, arg_derivation);
    
        /* Execute command */
        evaluate_boolean_process(pkgmgmt_set_profile((gchar*)arg_profile, (gchar*)arg_derivation, log_fd, log_fd), object, arg_pid, log_fd);
    }
    
    org_nixos_disnix_disnix_complete_set(object, invocation);
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
            
            g_free(derivations);
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

gboolean on_handle_query_requisites(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *const *arg_derivation)
{
    int log_fd = open_log_file(arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(object, arg_pid);
    }
    else
    {
        /* Print log entry */
        dprintf(log_fd, "Query requisites from derivations: ");
        print_paths(log_fd, (gchar**)arg_derivation);
        dprintf(log_fd, "\n");
        
        /* Execute command */
        evaluate_strv_process2(pkgmgmt_query_requisites((gchar**)arg_derivation, log_fd), object, arg_pid, log_fd);
    }
    
    org_nixos_disnix_disnix_complete_query_requisites(object, invocation);
    return TRUE;
}

/* Garbage collect method */

gboolean on_handle_collect_garbage(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, gboolean arg_delete_old)
{
    int log_fd = open_log_file(arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(object, arg_pid);
    }
    else
    {
        /* Print log entry */
        if(arg_delete_old)
            dprintf(log_fd, "Garbage collect and remove old derivations\n");
        else
            dprintf(log_fd, "Garbage collect\n");
    
        /* Execute command */
        evaluate_boolean_process(pkgmgmt_collect_garbage(arg_delete_old, log_fd, log_fd), object, arg_pid, log_fd);
    }
    
    org_nixos_disnix_disnix_complete_collect_garbage(object, invocation);
    return TRUE;
}

/* Common dysnomia invocation function */

static gboolean on_handle_dysnomia_activity(gchar *activity, OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_derivation, const gchar *arg_container, const gchar *arg_type, const gchar *const *arg_arguments)
{
    int log_fd = open_log_file(arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(object, arg_pid);
    }
    else
    {
        /* Print log entry */
        dprintf(log_fd, "%s: %s of type: %s in container: %s with arguments: ", activity, arg_derivation, arg_type, arg_container);
        print_paths(log_fd, (gchar**)arg_arguments);
        dprintf(log_fd, "\n");
    
        /* Execute command */
        evaluate_boolean_process(statemgmt_run_dysnomia_activity((gchar*)arg_type, activity, (gchar*)arg_derivation, (gchar*)arg_container, (gchar**)arg_arguments, log_fd, log_fd), object, arg_pid, log_fd);
    }
    
    org_nixos_disnix_disnix_complete_activate(object, invocation);
    return TRUE;
}

/* Activate method */

gboolean on_handle_activate(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_derivation, const gchar *arg_container, const gchar *arg_type, const gchar *const *arg_arguments)
{
    return on_handle_dysnomia_activity("activate", object, invocation, arg_pid, arg_derivation, arg_container, arg_type, arg_arguments);
}

/* Deactivate method */

gboolean on_handle_deactivate(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_derivation, const gchar *arg_container, const gchar *arg_type, const gchar *const *arg_arguments)
{
    return on_handle_dysnomia_activity("deactivate", object, invocation, arg_pid, arg_derivation, arg_container, arg_type, arg_arguments);
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
    return on_handle_dysnomia_activity("snapshot", object, invocation, arg_pid, arg_derivation, arg_container, arg_type, arg_arguments);
}

/* Restore method */

gboolean on_handle_restore(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_derivation, const gchar *arg_container, const gchar *arg_type, const gchar *const *arg_arguments)
{
    return on_handle_dysnomia_activity("restore", object, invocation, arg_pid, arg_derivation, arg_container, arg_type, arg_arguments);
}

/* Query all snapshots method */

gboolean on_handle_query_all_snapshots(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_container, const gchar *arg_component)
{
    int log_fd = open_log_file(arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(object, arg_pid);
    }
    else
    {
        /* Print log entry */
        dprintf(log_fd, "Query all snapshots from container: %s and component: %s\n", arg_container, arg_component);
        
        /* Execute command */
        evaluate_strv_process2(statemgmt_query_all_snapshots((gchar*)arg_container, (gchar*)arg_component, log_fd), object, arg_pid, log_fd);
    }
    
    org_nixos_disnix_disnix_complete_query_all_snapshots(object, invocation);
    return TRUE;
}

/* Query latest snapshot method */

gboolean on_handle_query_latest_snapshot(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_container, const gchar *arg_component)
{
    int log_fd = open_log_file(arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(object, arg_pid);
    }
    else
    {
        /* Print log entry */
        dprintf(log_fd, "Query latest snapshot from container: %s and component: %s\n", arg_container, arg_component);
    
        /* Execute command */
        evaluate_strv_process2(statemgmt_query_latest_snapshot((gchar*)arg_container, (gchar*)arg_component, log_fd), object, arg_pid, log_fd);
    }
    
    org_nixos_disnix_disnix_complete_query_latest_snapshot(object, invocation);
    return TRUE;
}

/* Query missing snapshots method */

gboolean on_handle_print_missing_snapshots(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *const *arg_component)
{
    int log_fd = open_log_file(arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(object, arg_pid);
    }
    else
    {
        /* Print log entry */
        dprintf(log_fd, "Print missing snapshots: ");
        print_paths(log_fd, (gchar**)arg_component);
        dprintf(log_fd, "\n");
        
        /* Execute command */
        evaluate_strv_process2(statemgmt_print_missing_snapshots((gchar**)arg_component, log_fd), object, arg_pid, log_fd);
    }
    
    org_nixos_disnix_disnix_complete_print_missing_snapshots(object, invocation);
    return TRUE;
}

/* Import snapshots operation */

gboolean on_handle_import_snapshots(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_container, const gchar *arg_component, const gchar *const *arg_snapshots)
{
    int log_fd = open_log_file(arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(object, arg_pid);
    }
    else
    {
        /* Print log entry */
        dprintf(log_fd, "Import snapshots: ");
        print_paths(log_fd, (gchar**)arg_snapshots);
        dprintf(log_fd, "\n");
        
        /* Execute command */
        evaluate_boolean_process(statemgmt_import_snapshots((gchar*)arg_container, (gchar*)arg_component, (gchar**)arg_snapshots, log_fd, log_fd), object, arg_pid, log_fd);
    }
    
    org_nixos_disnix_disnix_complete_import_snapshots(object, invocation);
    return TRUE;
}

/* Resolve snapshots operation */

gboolean on_handle_resolve_snapshots(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *const *arg_snapshots)
{
    int log_fd = open_log_file(arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(object, arg_pid);
    }
    else
    {
        /* Print log entry */
        dprintf(log_fd, "Resolve snapshots: ");
        print_paths(log_fd, (gchar**)arg_snapshots);
        dprintf(log_fd, "\n");
        
        /* Execute command */
        evaluate_strv_process2(statemgmt_resolve_snapshots((gchar**)arg_snapshots, log_fd), object, arg_pid, log_fd);
    }
    
    org_nixos_disnix_disnix_complete_resolve_snapshots(object, invocation);
    return TRUE;
}

/* Clean snapshots method */

gboolean on_handle_clean_snapshots(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, gint arg_keep, const gchar *arg_container, const char *arg_component)
{
    int log_fd = open_log_file(arg_pid);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile!\n");
        org_nixos_disnix_disnix_emit_failure(object, arg_pid);
    }
    else
    {
        /* Print log entry */
        dprintf(log_fd, "Clean old snapshots");
        
        if(g_strcmp0(arg_container, "") != 0)
            dprintf(log_fd, " for container: %s", arg_container);
        
        if(g_strcmp0(arg_component, "") != 0)
            dprintf(log_fd, " for component: %s", arg_component);
        
        dprintf(log_fd, " num of generations to keep: %d!\n", arg_keep);
        
        /* Execute command */
        evaluate_boolean_process(statemgmt_clean_snapshots(arg_keep, (gchar*)arg_container, (gchar*)arg_component, log_fd, log_fd), object, arg_pid, log_fd);
    }
    
    org_nixos_disnix_disnix_complete_clean_snapshots(object, invocation);
    return TRUE;
}

/* Delete state operation */

gboolean on_handle_delete_state(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_derivation, const gchar *arg_container, const gchar *arg_type, const gchar *const *arg_arguments)
{
    return on_handle_dysnomia_activity("collect-garbage", object, invocation, arg_pid, arg_derivation, arg_container, arg_type, arg_arguments);
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
