/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2022  Sander van der Burg
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

#include "disnix-client.h"
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "disnix-dbus.h"
#define BUFFER_SIZE 1024

char *logdir;

static void print_log(const gint pid)
{
    char pidStr[15], buf[BUFFER_SIZE];
    gchar *logfile;
    FILE *file;

    sprintf(pidStr, "%d", pid);
    logfile = g_strconcat(logdir, "/", pidStr, NULL);
    file = fopen(logfile, "r");

    if(file == NULL)
        fprintf(stderr, "Cannot display logfile of pid: %d\n", pid);
    else
    {
        size_t bytes_read;

        while(!feof(file) && (bytes_read = fread(buf, 1, BUFFER_SIZE, file)) > 0)
            fwrite(buf, 1, bytes_read, stderr);

        fclose(file);
    }

    g_free(logfile);
}

/* Signal handlers */

static void disnix_finish_signal_handler(GDBusProxy *proxy, const gint pid, gpointer user_data)
{
    gint my_pid = *((gint*)user_data);

    if(pid == my_pid)
        exit(0);
}

static void disnix_success_signal_handler(GDBusProxy *proxy, const gint pid, gchar **paths, gpointer user_data)
{
    gint my_pid = *((gint*)user_data);

    if(pid == my_pid)
    {
        unsigned int i;

        for(i = 0; i < g_strv_length(paths); i++)
            g_print("%s\n", paths[i]);

        exit(0);
    }
}

static void disnix_failure_signal_handler(GDBusProxy *proxy, const gint pid, gpointer user_data)
{
    gint my_pid = *((gint*)user_data);

    if(pid == my_pid)
    {
        print_log(pid);
        exit(1);
    }
}

static void cleanup(OrgNixosDisnixDisnix *proxy, gchar **paths, gchar **arguments)
{
    g_free(logdir);
    g_strfreev(paths);
    g_strfreev(arguments);
    if(proxy != NULL)
        g_object_unref(proxy);
}

static gchar *check_dysnomia_activity_parameters(OrgNixosDisnixDisnix *proxy, gchar *type, gchar **paths, gchar *container, gchar **arguments)
{
    if(type == NULL)
    {
        g_printerr("ERROR: A type must be specified!\n");
        cleanup(proxy, paths, arguments);
        return NULL;
    }
    else if(paths[0] == NULL)
    {
        g_printerr("ERROR: A Nix store component has to be specified!\n");
        cleanup(proxy, paths, arguments);
        return NULL;
    }

    if(container == NULL)
        return type;
    else
        return container;
}

int run_disnix_client(Operation operation, gchar **paths, const unsigned int flags, char *profile, gchar **arguments, char *type, char *container, char *component, int keep)
{
    /* Proxy object representing the D-Bus service object. */
    OrgNixosDisnixDisnix *proxy;

    /* GMainLoop object */
    GMainLoop *mainloop;

    /* Captures the results of D-Bus operations */
    GError *error = NULL;

    /* Other declarations */
    gint pid;

    /* If no operation is specified we should quit */
    if(operation == OP_NONE)
    {
        g_printerr("No operation has been specified!\n");
        cleanup(NULL, paths, arguments);
        return 1;
    }

    /* Connect to the session/system bus */

    GBusType bus_type;

    if(flags & FLAG_SESSION_BUS)
        bus_type = G_BUS_TYPE_SESSION;
    else
        bus_type = G_BUS_TYPE_SYSTEM;

    proxy = org_nixos_disnix_disnix_proxy_new_for_bus_sync(bus_type,
        G_DBUS_PROXY_FLAGS_NONE,
        "org.nixos.disnix.Disnix",
        "/org/nixos/disnix/Disnix",
        NULL,
        &error);

    if(error != NULL)
    {
        if(flags & FLAG_SESSION_BUS)
            g_printerr("Cannot connect to session bus! Reason: %s\n", error->message);
        else
            g_printerr("Cannot connect to system bus! Reason: %s\n", error->message);

        g_error_free(error);
        return 1;
    }

    /* Register the signatures for the signal handlers */
    g_signal_connect(proxy, "finish", G_CALLBACK(disnix_finish_signal_handler), &pid);
    g_signal_connect(proxy, "success", G_CALLBACK(disnix_success_signal_handler), &pid);
    g_signal_connect(proxy, "failure", G_CALLBACK(disnix_failure_signal_handler), &pid);

    /* Receive the logdir */
    org_nixos_disnix_disnix_call_get_logdir_sync(proxy, &logdir, NULL, &error);

    if(error != NULL)
    {
        g_printerr("ERROR: Cannot obtain log directory! Reason: %s\n", error->message);
        cleanup(proxy, paths, arguments);
        g_error_free(error);
        return 1;
    }

    /* Receive a PID for the job we want to execute */
    org_nixos_disnix_disnix_call_get_job_id_sync(proxy, &pid, NULL, &error);

    if(error != NULL)
    {
        g_printerr("ERROR: Cannot obtain job id! Reason: %s\n", error->message);
        cleanup(proxy, paths, arguments);
        g_error_free(error);
        return 1;
    }

    /* Execute operation */

    switch(operation)
    {
        case OP_IMPORT:
            if(paths[0] == NULL)
            {
                g_printerr("ERROR: A Nix store component has to be specified!\n");
                cleanup(proxy, paths, arguments);
                return 1;
            }
            else
                org_nixos_disnix_disnix_call_import_sync(proxy, pid, paths[0], NULL, &error);
            break;
        case OP_EXPORT:
            org_nixos_disnix_disnix_call_export_sync(proxy, pid, (const gchar**) paths, NULL, &error);
            break;
        case OP_PRINT_INVALID:
            org_nixos_disnix_disnix_call_print_invalid_sync(proxy, pid, (const gchar**) paths, NULL, &error);
            break;
        case OP_REALISE:
            org_nixos_disnix_disnix_call_realise_sync(proxy, pid, (const gchar**) paths, NULL, &error);
            break;
        case OP_SET:
            if(paths[0] == NULL)
            {
                g_printerr("ERROR: A Nix store component has to be specified!\n");
                cleanup(proxy, paths, arguments);
                return 1;
            }
            else
                org_nixos_disnix_disnix_call_set_sync(proxy, pid, profile, paths[0], NULL, &error);
            break;
        case OP_QUERY_INSTALLED:
            org_nixos_disnix_disnix_call_query_installed_sync(proxy, pid, profile, NULL, &error);
            break;
        case OP_QUERY_REQUISITES:
            org_nixos_disnix_disnix_call_query_requisites_sync(proxy, pid, (const gchar**) paths, NULL, &error);
            break;
        case OP_COLLECT_GARBAGE:
            org_nixos_disnix_disnix_call_collect_garbage_sync(proxy, pid, (flags & FLAG_DELETE_OLD), NULL, &error);
            break;
        case OP_ACTIVATE:
            container = check_dysnomia_activity_parameters(proxy, type, paths, container, arguments);

            if(container != NULL)
                org_nixos_disnix_disnix_call_activate_sync(proxy, pid, paths[0], container, type, (const gchar**) arguments, NULL, &error);
            break;
        case OP_DEACTIVATE:
            container = check_dysnomia_activity_parameters(proxy, type, paths, container, arguments);

            if(container != NULL)
                org_nixos_disnix_disnix_call_deactivate_sync(proxy, pid, paths[0], container, type, (const gchar**) arguments, NULL, &error);
            break;
        case OP_DELETE_STATE:
            container = check_dysnomia_activity_parameters(proxy, type, paths, container, arguments);

            if(container != NULL)
                org_nixos_disnix_disnix_call_delete_state_sync(proxy, pid, paths[0], container, type, (const gchar**) arguments, NULL, &error);
            break;
        case OP_SNAPSHOT:
            container = check_dysnomia_activity_parameters(proxy, type, paths, container, arguments);

            if(container != NULL)
                org_nixos_disnix_disnix_call_snapshot_sync(proxy, pid, paths[0], container, type, (const gchar**) arguments, NULL, &error);
            break;
        case OP_RESTORE:
            container = check_dysnomia_activity_parameters(proxy, type, paths, container, arguments);

            if(container != NULL)
                org_nixos_disnix_disnix_call_restore_sync(proxy, pid, paths[0], container, type, (const gchar**) arguments, NULL, &error);
            break;
        case OP_LOCK:
            org_nixos_disnix_disnix_call_lock_sync(proxy, pid, profile, NULL, &error);
            break;
        case OP_UNLOCK:
            org_nixos_disnix_disnix_call_unlock_sync(proxy, pid, profile, NULL, &error);
            break;
        case OP_QUERY_ALL_SNAPSHOTS:
            org_nixos_disnix_disnix_call_query_all_snapshots_sync(proxy, pid, container, component, NULL, &error);
            break;
        case OP_QUERY_LATEST_SNAPSHOT:
            org_nixos_disnix_disnix_call_query_latest_snapshot_sync(proxy, pid, container, component, NULL, &error);
            break;
        case OP_PRINT_MISSING_SNAPSHOTS:
            org_nixos_disnix_disnix_call_print_missing_snapshots_sync(proxy, pid, (const gchar**) paths, NULL, &error);
            break;
        case OP_IMPORT_SNAPSHOTS:
            if(paths[0] == NULL)
            {
                g_printerr("ERROR: A Dysnomia snapshot has to be specified!\n");
                cleanup(proxy, paths, arguments);
                return 1;
            }
            else
                org_nixos_disnix_disnix_call_import_snapshots_sync(proxy, pid, container, component, (const gchar**) paths, NULL, &error);
            break;
        case OP_RESOLVE_SNAPSHOTS:
            if(paths[0] == NULL)
            {
                g_printerr("ERROR: A Dysnomia snapshot has to be specified!\n");
                cleanup(proxy, paths, arguments);
                return 1;
            }
            else
                org_nixos_disnix_disnix_call_resolve_snapshots_sync(proxy, pid, (const gchar**) paths, NULL, &error);
            break;
        case OP_CLEAN_SNAPSHOTS:
            if(container == NULL)
                container = "";

            if(component == NULL)
                component = "";

            org_nixos_disnix_disnix_call_clean_snapshots_sync(proxy, pid, keep, container, component, NULL, &error);
            break;
        case OP_CAPTURE_CONFIG:
            org_nixos_disnix_disnix_call_capture_config_sync(proxy, pid, NULL, &error);
            break;
        case OP_SHELL:
            g_printerr("ERROR: This operation is unsupported by this client!\n");
            cleanup(proxy, paths, arguments);
            return 1;
            break;
        case OP_NONE:
            g_printerr("ERROR: No operation specified!\n");
            cleanup(proxy, paths, arguments);
            return 1;
    }

    if(error != NULL)
    {
        g_printerr("Error while executing the operation! Reason: %s\n", error->message);
        cleanup(proxy, paths, arguments);
        g_error_free(error);
        return 1;
    }

    /* Create main loop */
    mainloop = g_main_loop_new(NULL, FALSE);
    if(mainloop == NULL)
    {
        g_printerr("Cannot create main loop.\n");
        cleanup(proxy, paths, arguments);
        return 1;
    }

    /* Run loop and wait for signals */
    g_main_loop_run(mainloop);

    /* Operation is finished */
    cleanup(proxy, paths, arguments);

    return 1;
}
