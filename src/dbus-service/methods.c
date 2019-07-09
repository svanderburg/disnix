/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2018  Sander van der Burg
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
#include <glib.h>
#include "logging.h"
#include "locking.h"
#include "jobmanagement.h"
#include "signaling.h"
#include "package-management.h"
#include "state-management.h"

extern char *tmpdir, *logdir;

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
    int log_fd = open_log_file(object, arg_pid);

    if(log_fd != -1)
    {
        /* Print log entry */
        dprintf(log_fd, "Importing: %s\n", arg_closure);

        /* Execute command */
        signal_boolean_result(pkgmgmt_import_closure(arg_closure, log_fd, log_fd), object, arg_pid, log_fd);
    }

    org_nixos_disnix_disnix_complete_import(object, invocation);
    return TRUE;
}

/* Export method */

gboolean on_handle_export(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *const *arg_derivation)
{
    int log_fd = open_log_file(object, arg_pid);

    if(log_fd != -1)
    {
        pid_t pid;
        int temp_fd;
        gchar *tempfilename;

        /* Print log entry */
        dprintf(log_fd, "Exporting: ");
        print_paths(log_fd, (gchar**)arg_derivation);
        dprintf(log_fd, "\n");

        /* Execute command */
        tempfilename = pkgmgmt_export_closure(tmpdir, (gchar**)arg_derivation, log_fd, &pid, &temp_fd);
        signal_tempfile_result(pid, tempfilename, temp_fd, object, arg_pid, log_fd);
    }

    org_nixos_disnix_disnix_complete_export(object, invocation);
    return TRUE;
}

/* Print invalid paths method */

gboolean on_handle_print_invalid(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *const *arg_derivation)
{
    int log_fd = open_log_file(object, arg_pid);

    if(log_fd != -1)
    {
        /* Print log entry */
        dprintf(log_fd, "Print invalid: ");
        print_paths(log_fd, (gchar**)arg_derivation);
        dprintf(log_fd, "\n");

        /* Execute command */
        signal_strv_result(pkgmgmt_print_invalid_packages((gchar**)arg_derivation, log_fd), object, arg_pid, log_fd);
    }

    org_nixos_disnix_disnix_complete_print_invalid(object, invocation);
    return TRUE;
}

/* Realise method */

gboolean on_handle_realise(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *const *arg_derivation)
{
    int log_fd = open_log_file(object, arg_pid);

    if(log_fd != -1)
    {
        /* Print log entry */
        dprintf(log_fd, "Realising: ");
        print_paths(log_fd, (gchar**)arg_derivation);
        dprintf(log_fd, "\n");

        /* Execute command and wait and asychronously propagate its end result */
        signal_strv_result(pkgmgmt_realise((gchar**)arg_derivation, log_fd), object, arg_pid, log_fd);
    }

    org_nixos_disnix_disnix_complete_realise(object, invocation);
    return TRUE;
}

/* Set method */

gboolean on_handle_set(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_profile, const gchar *arg_derivation)
{
    int log_fd = open_log_file(object, arg_pid);

    if(log_fd != -1)
    {
        /* Print log entry */
        dprintf(log_fd, "Set profile: %s with derivation: %s\n", arg_profile, arg_derivation);

        /* Execute command */
        signal_boolean_result(pkgmgmt_set_profile((gchar*)arg_profile, (gchar*)arg_derivation, log_fd, log_fd), object, arg_pid, log_fd);
    }

    org_nixos_disnix_disnix_complete_set(object, invocation);
    return TRUE;
}

/* Query installed method */

gboolean on_handle_query_installed(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_profile)
{
    int log_fd = open_log_file(object, arg_pid);

    if(log_fd != -1)
    {
        /* Print log entry */
        dprintf(log_fd, "Query installed derivations from profile: %s\n", arg_profile);

        /* Execute command */
        signal_string_result(query_installed_services(LOCALSTATEDIR, (gchar*)arg_profile), object, arg_pid, log_fd);
    }

    org_nixos_disnix_disnix_complete_query_installed(object, invocation);
    return TRUE;
}

/* Query requisites method */

gboolean on_handle_query_requisites(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *const *arg_derivation)
{
    int log_fd = open_log_file(object, arg_pid);

    if(log_fd != -1)
    {
        /* Print log entry */
        dprintf(log_fd, "Query requisites from derivations: ");
        print_paths(log_fd, (gchar**)arg_derivation);
        dprintf(log_fd, "\n");

        /* Execute command */
        signal_strv_result(pkgmgmt_query_requisites((gchar**)arg_derivation, log_fd), object, arg_pid, log_fd);
    }

    org_nixos_disnix_disnix_complete_query_requisites(object, invocation);
    return TRUE;
}

/* Garbage collect method */

gboolean on_handle_collect_garbage(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, gboolean arg_delete_old)
{
    int log_fd = open_log_file(object, arg_pid);

    if(log_fd != -1)
    {
        /* Print log entry */
        if(arg_delete_old)
            dprintf(log_fd, "Garbage collect and remove old derivations\n");
        else
            dprintf(log_fd, "Garbage collect\n");

        /* Execute command */
        signal_boolean_result(pkgmgmt_collect_garbage(arg_delete_old, log_fd, log_fd), object, arg_pid, log_fd);
    }

    org_nixos_disnix_disnix_complete_collect_garbage(object, invocation);
    return TRUE;
}

/* Common dysnomia invocation function */

static gboolean on_handle_dysnomia_activity(gchar *activity, OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_derivation, const gchar *arg_container, const gchar *arg_type, const gchar *const *arg_arguments)
{
    int log_fd = open_log_file(object, arg_pid);

    if(log_fd != -1)
    {
        /* Print log entry */
        dprintf(log_fd, "%s: %s of type: %s in container: %s with arguments: ", activity, arg_derivation, arg_type, arg_container);
        print_paths(log_fd, (gchar**)arg_arguments);
        dprintf(log_fd, "\n");

        /* Execute command */
        signal_boolean_result(statemgmt_run_dysnomia_activity((gchar*)arg_type, activity, (gchar*)arg_derivation, (gchar*)arg_container, (gchar**)arg_arguments, log_fd, log_fd), object, arg_pid, log_fd);
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

gboolean on_handle_lock(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_profile)
{
    int log_fd = open_log_file(object, arg_pid);

    if(log_fd != -1)
    {
        ProfileManifest *profile_manifest;

        /* Print log entry */
        dprintf(log_fd, "Acquiring lock on profile: %s\n", arg_profile);

        /* Lock the disnix instance */
        profile_manifest = create_profile_manifest_from_current_deployment(LOCALSTATEDIR, (gchar*)arg_profile);

        if(profile_manifest == NULL)
        {
            dprintf(log_fd, "Corrupt profile manifest: a service or type is missing!\n");
            org_nixos_disnix_disnix_emit_failure(object, arg_pid);
        }
        else
        {
            signal_boolean_result(acquire_locks_async(log_fd, tmpdir, profile_manifest, (gchar*)arg_profile), object, arg_pid, log_fd);

            /* Cleanup */
            delete_profile_manifest(profile_manifest);
        }
    }

    org_nixos_disnix_disnix_complete_lock(object, invocation);
    return TRUE;
}

/* Unlock method */

gboolean on_handle_unlock(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_profile)
{
    int log_fd = open_log_file(object, arg_pid);

    if(log_fd != -1)
    {
        ProfileManifest *profile_manifest;

        /* Print log entry */
        dprintf(log_fd, "Releasing lock on profile: %s\n", arg_profile);

        /* Unlock the Disnix instance */
        profile_manifest = create_profile_manifest_from_current_deployment(LOCALSTATEDIR, (gchar*)arg_profile);
        signal_boolean_result(release_locks_async(log_fd, tmpdir, profile_manifest, (gchar*)arg_profile), object, arg_pid, log_fd);

       /* Cleanup */
       delete_profile_manifest(profile_manifest);
    }

    org_nixos_disnix_disnix_complete_unlock(object, invocation);
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
    int log_fd = open_log_file(object, arg_pid);

    if(log_fd != -1)
    {
        /* Print log entry */
        dprintf(log_fd, "Query all snapshots from container: %s and component: %s\n", arg_container, arg_component);

        /* Execute command */
        signal_strv_result(statemgmt_query_all_snapshots((gchar*)arg_container, (gchar*)arg_component, log_fd), object, arg_pid, log_fd);
    }

    org_nixos_disnix_disnix_complete_query_all_snapshots(object, invocation);
    return TRUE;
}

/* Query latest snapshot method */

gboolean on_handle_query_latest_snapshot(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_container, const gchar *arg_component)
{
    int log_fd = open_log_file(object, arg_pid);

    if(log_fd != -1)
    {
        /* Print log entry */
        dprintf(log_fd, "Query latest snapshot from container: %s and component: %s\n", arg_container, arg_component);

        /* Execute command */
        signal_strv_result(statemgmt_query_latest_snapshot((gchar*)arg_container, (gchar*)arg_component, log_fd), object, arg_pid, log_fd);
    }

    org_nixos_disnix_disnix_complete_query_latest_snapshot(object, invocation);
    return TRUE;
}

/* Query missing snapshots method */

gboolean on_handle_print_missing_snapshots(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *const *arg_component)
{
    int log_fd = open_log_file(object, arg_pid);

    if(log_fd != -1)
    {
        /* Print log entry */
        dprintf(log_fd, "Print missing snapshots: ");
        print_paths(log_fd, (gchar**)arg_component);
        dprintf(log_fd, "\n");

        /* Execute command */
        signal_strv_result(statemgmt_print_missing_snapshots((gchar**)arg_component, log_fd), object, arg_pid, log_fd);
    }

    org_nixos_disnix_disnix_complete_print_missing_snapshots(object, invocation);
    return TRUE;
}

/* Import snapshots operation */

gboolean on_handle_import_snapshots(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_container, const gchar *arg_component, const gchar *const *arg_snapshots)
{
    int log_fd = open_log_file(object, arg_pid);

    if(log_fd != -1)
    {
        /* Print log entry */
        dprintf(log_fd, "Import snapshots: ");
        print_paths(log_fd, (gchar**)arg_snapshots);
        dprintf(log_fd, "\n");

        /* Execute command */
        signal_boolean_result(statemgmt_import_snapshots((gchar*)arg_container, (gchar*)arg_component, (gchar**)arg_snapshots, log_fd, log_fd), object, arg_pid, log_fd);
    }

    org_nixos_disnix_disnix_complete_import_snapshots(object, invocation);
    return TRUE;
}

/* Resolve snapshots operation */

gboolean on_handle_resolve_snapshots(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *const *arg_snapshots)
{
    int log_fd = open_log_file(object, arg_pid);

    if(log_fd != -1)
    {
        /* Print log entry */
        dprintf(log_fd, "Resolve snapshots: ");
        print_paths(log_fd, (gchar**)arg_snapshots);
        dprintf(log_fd, "\n");

        /* Execute command */
        signal_strv_result(statemgmt_resolve_snapshots((gchar**)arg_snapshots, log_fd), object, arg_pid, log_fd);
    }

    org_nixos_disnix_disnix_complete_resolve_snapshots(object, invocation);
    return TRUE;
}

/* Clean snapshots method */

gboolean on_handle_clean_snapshots(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, gint arg_keep, const gchar *arg_container, const char *arg_component)
{
    int log_fd = open_log_file(object, arg_pid);

    if(log_fd != -1)
    {
        /* Print log entry */
        dprintf(log_fd, "Clean old snapshots");

        if(g_strcmp0(arg_container, "") != 0)
            dprintf(log_fd, " for container: %s", arg_container);

        if(g_strcmp0(arg_component, "") != 0)
            dprintf(log_fd, " for component: %s", arg_component);

        dprintf(log_fd, " num of generations to keep: %d!\n", arg_keep);

        /* Execute command */
        signal_boolean_result(statemgmt_clean_snapshots(arg_keep, (gchar*)arg_container, (gchar*)arg_component, log_fd, log_fd), object, arg_pid, log_fd);
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

gboolean on_handle_capture_config(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid)
{
    int log_fd = open_log_file(object, arg_pid);

    if(log_fd != -1)
    {
        pid_t pid;
        int temp_fd;
        gchar *tempfilename = statemgmt_capture_config(tmpdir, log_fd, &pid, &temp_fd);

        signal_tempfile_result(pid, tempfilename, temp_fd, object, arg_pid, log_fd);
    }

    org_nixos_disnix_disnix_complete_capture_config(object, invocation);
    return TRUE;
}
