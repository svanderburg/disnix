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

#include "disnix-service.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <glib.h>
#include <glib-unix.h>
#include "disnix-dbus.h"
#include "jobmanagement.h"
#include "logging.h"
#include "methods.h"
#include "daemonize.h"

/* Server settings variables */

/** Path to the temp directory */
char *tmpdir;

/* Path to the log directory */
extern char *logdir;

typedef struct
{
    /* Indicates whether we want to connect to the session bus or system bus */
    ProcReact_bool session_bus;
    /* Path where Disnix should should store logs of the activities that it executes */
    char *log_path;
    /* GLib mainloop that keeps the server running */
    GMainLoop *mainloop;
    /* ID of the bus owner */
    guint owner_id;
    /* File where the daemon should write general log messages */
    FILE *log_fd;
    /* Pid file of the daemon or NULL if it the services runs in foreground mode */
    char *pid_file;
}
DaemonData;

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
    g_signal_connect(interface, "handle-capture-config", G_CALLBACK(on_handle_capture_config), NULL);

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
    DaemonData *data = (DaemonData*)user_data;

    fprintf(data->log_fd, "Name lost: %s\n", name);

    if (connection == NULL)
        fprintf(data->log_fd, "No D-Bus connection has been established!\n");
    else
        fprintf(data->log_fd, "A D-Bus connection seems to have been established!\n");

    exit(1);
}

static void configure_tmp_dir(void)
{
    tmpdir = getenv("TMPDIR");

    if(tmpdir == NULL)
        tmpdir = "/tmp";
}

static gboolean handle_termination(gpointer user_data)
{
    GMainLoop *mainloop = (GMainLoop*)user_data;
    g_main_loop_quit(mainloop);
    return TRUE;
}

static ProcReact_bool initialize_disnix_service(void *data)
{
    DaemonData *daemon_data = (DaemonData*)data;

    /* Determine the temp directory */
    configure_tmp_dir();

    /* Determine the log directory */
    set_logdir(daemon_data->log_path);

    /* Create a GMainloop with initial state of 'not running' (FALSE) */
    daemon_data->mainloop = g_main_loop_new(NULL, FALSE);
    if(daemon_data->mainloop == NULL)
    {
        fprintf(daemon_data->log_fd, "ERROR: Failed to create the mainloop!\n");
        return FALSE;
    }

    /* Figure out what the next job id number is */
    determine_next_pid(logdir);

    /* Connect to the system/session bus */
    GBusType bus_type;

    if(daemon_data->session_bus)
        bus_type = G_BUS_TYPE_SESSION;
    else
        bus_type = G_BUS_TYPE_SYSTEM;

    daemon_data->owner_id = g_bus_own_name(bus_type,
        "org.nixos.disnix.Disnix",
        G_BUS_NAME_OWNER_FLAGS_NONE,
        on_bus_acquired,
        on_name_acquired,
        on_name_lost,
        daemon_data,
        NULL);

    if(daemon_data->pid_file != NULL)
    {
        g_unix_signal_add(SIGTERM, handle_termination, daemon_data->mainloop);
        g_unix_signal_add(SIGINT, handle_termination, daemon_data->mainloop);
    }

    return TRUE;
}

static int start_disnix_service(void *data)
{
    DaemonData *daemon_data = (DaemonData*)data;

    /* Starting the main loop */
    fprintf(daemon_data->log_fd, "The Disnix service is running!\n");
    fflush(daemon_data->log_fd);

    g_main_loop_run(daemon_data->mainloop);

    g_bus_unown_name(daemon_data->owner_id);

    if(daemon_data->pid_file == NULL)
        return 1; /* The main loop should not be stopped in foreground mode, but if it does, then return the exit failure status */
    else
    {
        unlink(daemon_data->pid_file);
        return 0;
    }
}

int start_disnix_service_foreground(ProcReact_bool session_bus, char *log_path)
{
    int exit_status;
    DaemonData *data = (DaemonData*)g_malloc(sizeof(DaemonData)); /* We must allocate the daemon data on the heap -> it is required by callback functions invoked from a thread */
    data->session_bus = session_bus;
    data->log_path = log_path;
    data->log_fd = stderr;
    data->pid_file = NULL;

    exit_status = !(initialize_disnix_service(data) && (start_disnix_service(data) == 0));

    g_free(data);
    return exit_status;
}

int start_disnix_service_daemon(ProcReact_bool session_bus, char *log_path, char *pid_file, char *log_file)
{
    FILE *log_fd = fopen(log_file, "a");

    if(log_fd == NULL)
    {
        fprintf(stderr, "Cannot open log file: %s for writing!\n", log_file);
        return 1;
    }
    else
    {
        DaemonData *data = (DaemonData*)g_malloc(sizeof(DaemonData)); /* We must allocate the daemon data on the heap -> it is required by callback functions invoked from a thread */
        data->session_bus = session_bus;
        data->log_path = log_path;
        data->log_fd = log_fd;
        data->pid_file = pid_file;

        DaemonStatus status = daemonize(pid_file, data, initialize_disnix_service, start_disnix_service);
        print_daemon_status(status, stderr);

        g_free(data);
        fclose(log_fd);
        return status;
    }
}
