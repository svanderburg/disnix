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

#include "disnix-service.h"
#include <unistd.h>
#include <stdlib.h>
#include <glib.h>
#include "disnix-dbus.h"
#include "jobmanagement.h"
#include "methods.h"

/* Server settings variables */

/** Path to the temp directory */
char *tmpdir;

/* Path to the log directory */
extern char *logdir;

static void on_bus_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
    GError *error = NULL;
    /* Create interface skeleton */
    OrgNixosDisnixDisnix *interface = org_nixos_disnix_disnix_skeleton_new();
    
    /* Register a signal for each method */
    g_signal_connect(interface, "handle-get-job-id", G_CALLBACK(on_handle_get_job_id), NULL);
    g_signal_connect(interface, "handle-import", G_CALLBACK(on_handle_import), NULL);
    g_signal_connect(interface, "handle-export", G_CALLBACK(on_handle_export), NULL);
    g_signal_connect(interface, "handle-print-invalid", G_CALLBACK(on_handle_print_invalid), NULL);
    g_signal_connect(interface, "handle-realise", G_CALLBACK(on_handle_realise), NULL);
    g_signal_connect(interface, "handle-set", G_CALLBACK(on_handle_set), NULL);
    g_signal_connect(interface, "handle-query-installed", G_CALLBACK(on_handle_query_installed), NULL);
    g_signal_connect(interface, "handle-query-requisites", G_CALLBACK(on_handle_query_requisites), NULL);
    g_signal_connect(interface, "handle-collect-garbage", G_CALLBACK(on_handle_collect_garbage), NULL);
    g_signal_connect(interface, "handle-activate", G_CALLBACK(on_handle_activate), NULL);
    g_signal_connect(interface, "handle-deactivate", G_CALLBACK(on_handle_deactivate), NULL);
    g_signal_connect(interface, "handle-lock", G_CALLBACK(on_handle_lock), NULL);
    g_signal_connect(interface, "handle-unlock", G_CALLBACK(on_handle_unlock), NULL);
    g_signal_connect(interface, "handle-delete-state", G_CALLBACK(on_handle_delete_state), NULL);
    g_signal_connect(interface, "handle-snapshot", G_CALLBACK(on_handle_snapshot), NULL);
    g_signal_connect(interface, "handle-restore", G_CALLBACK(on_handle_restore), NULL);
    g_signal_connect(interface, "handle-query-all-snapshots", G_CALLBACK(on_handle_query_all_snapshots), NULL);
    g_signal_connect(interface, "handle-query-latest-snapshot", G_CALLBACK(on_handle_query_latest_snapshot), NULL);
    g_signal_connect(interface, "handle-print-missing-snapshots", G_CALLBACK(on_handle_print_missing_snapshots), NULL);
    g_signal_connect(interface, "handle-import-snapshots", G_CALLBACK(on_handle_import_snapshots), NULL);
    g_signal_connect(interface, "handle-resolve-snapshots", G_CALLBACK(on_handle_resolve_snapshots), NULL);
    g_signal_connect(interface, "handle-clean-snapshots", G_CALLBACK(on_handle_clean_snapshots), NULL);
    g_signal_connect(interface, "handle-get-logdir", G_CALLBACK(on_handle_get_logdir), NULL);
    
    /* Export skeleton */
    if(!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(interface),
        connection,
        "/org/nixos/disnix/Disnix",
        &error))
    {
        g_printerr("Cannot export interface skeleton!\n");
        g_error_free(error);
        exit(1);
    }
}

static void on_name_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
}

static void on_name_lost(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
    exit(1);
}

int start_disnix_service(int session_bus, char *log_path)
{
    /* GLib mainloop that keeps the server running */
    GMainLoop *mainloop;
    
    /* ID of the bus owner */
    guint owner_id;
    
    /* Determine the temp directory */
    tmpdir = getenv("TMPDIR");
    
    if(tmpdir == NULL)
	tmpdir = "/tmp";
    
    /* Determine the log directory */
    logdir = log_path;
    
    /* Create a GMainloop with initial state of 'not running' (FALSE) */
    mainloop = g_main_loop_new(NULL, FALSE);
    if(mainloop == NULL)
    {
	g_printerr("ERROR: Failed to create the mainloop!\n");
	return 1;
    }
    
    /* Figure out what the next job id number is */
    determine_next_pid(logdir);
    
    /* Connect to the system/session bus */
    if(session_bus)
    {
        owner_id = g_bus_own_name(G_BUS_TYPE_SESSION,
            "org.nixos.disnix.Disnix",
            G_BUS_NAME_OWNER_FLAGS_NONE,
            on_bus_acquired,
            on_name_acquired,
            on_name_lost,
            NULL,
            NULL);
    }
    else
    {
        owner_id = g_bus_own_name(G_BUS_TYPE_SYSTEM,
            "org.nixos.disnix.Disnix",
            G_BUS_NAME_OWNER_FLAGS_NONE,
            on_bus_acquired,
            on_name_acquired,
            on_name_lost,
            NULL,
            NULL);
    }
    
    /* Starting the main loop */
    g_print("The Disnix service is running!\n");
    g_main_loop_run(mainloop);
    
    g_bus_unown_name(owner_id);
    
    /* The main loop should not be stopped, but if it does return the exit failure status */
    return 1;
}
