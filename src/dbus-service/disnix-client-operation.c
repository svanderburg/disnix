/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2015  Sander van der Burg
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

#include "disnix-client-operation.h"
#include <stdlib.h>
#include "disnix-client.h"
#include "disnix-marshal.h"

/* Signal handlers */

static void disnix_finish_signal_handler(DBusGProxy *proxy, const gint pid, gpointer user_data)
{
    gint my_pid = *((gint*)user_data);

    g_printerr("Received finish signal from pid: %d\n", pid);

    /* Stop the main loop if our job is done */
    if(pid == my_pid)
	exit(0);
}

static void disnix_success_signal_handler(DBusGProxy *proxy, const gint pid, gchar **derivation, gpointer user_data)
{
    unsigned int i;
    gint my_pid = *((gint*)user_data);
    
    g_printerr("Received success signal from pid: %d\n", pid);
    
    for(i = 0; i < g_strv_length(derivation); i++)
	g_print("%s", derivation[i]);
	
    /* Stop the main loop if our job is done */
    if(pid == my_pid)
	exit(0);
}

static void disnix_failure_signal_handler(DBusGProxy *proxy, const gint pid, gpointer user_data)
{
    gint my_pid = *((gint*)user_data);
    
    g_printerr("Received failure signal from pid: %d\n", pid);
    
    /* Stop the main loop if our job is done */
    if(pid == my_pid)
	exit(1);
}

static void cleanup(gchar **derivation, gchar **arguments)
{
    g_strfreev(derivation);
    g_strfreev(arguments);
}

int run_disnix_client(Operation operation, gchar **derivation, gboolean session_bus, char *profile, gboolean delete_old, gchar **arguments, char *type, char *container, char *component)
{
    /* The GObject representing a D-Bus connection. */
    DBusGConnection *bus;
    
    /* Proxy object representing the D-Bus service object. */
    DBusGProxy *remote_object;

    /* GMainLoop object */
    GMainLoop *mainloop;
    
    /* Captures the results of D-Bus operations */
    GError *error = NULL;
    
    /* Other declarations */
    gint pid;
    
    /* If no operation is specified we should quit */
    if(operation == OP_NONE)
    {
	g_printerr("No operation is specified!\n");
	cleanup(derivation, arguments);
	return 1;
    }
    
    /* Create main loop */
    mainloop = g_main_loop_new(NULL, FALSE);
    if(mainloop == NULL)
    {
	g_printerr("Cannot create main loop.\n");
	cleanup(derivation, arguments);
	return 1;
    }

    /* Connect to the session/system bus */
    
    if(session_bus)
    {
	g_printerr("Connecting to the session bus.\n");
	bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
    }
    else
    {
	g_printerr("Connecting to the system bus.\n");
	bus = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);
    }
    
    if(!bus)
    {
        g_printerr("Cannot connect to session/system bus! Reason: %s\n", error->message);
        return 1;
    }
    
    /* Create the proxy object that will be used to access the object */
    
    g_printerr("Creating a Glib proxy object.\n");
    remote_object = dbus_g_proxy_new_for_name (bus,
        "org.nixos.disnix.Disnix", /* name */
        "/org/nixos/disnix/Disnix", /* Object path */
       "org.nixos.disnix.Disnix"); /* Interface */

    if(remote_object == NULL)
    {
        g_printerr("Cannot create the proxy object! Reason: %s\n", error->message);
        cleanup(derivation, arguments);
        return 1;
    }

    /* Register marshaller for the return values of the success signal */
    dbus_g_object_register_marshaller(marshal_VOID__INT_BOXED, G_TYPE_NONE, G_TYPE_INT, G_TYPE_STRV, G_TYPE_INVALID);
    
    /* Register the signatures for the signal handlers */

    g_printerr("Add the argument signatures for the signal handler\n");
    dbus_g_proxy_add_signal(remote_object, "finish", G_TYPE_INT, G_TYPE_INVALID);
    dbus_g_proxy_add_signal(remote_object, "success", G_TYPE_INT, G_TYPE_STRV, G_TYPE_INVALID);
    dbus_g_proxy_add_signal(remote_object, "failure", G_TYPE_INT, G_TYPE_INVALID);

    g_printerr("Register D-Bus signal handlers\n");
    dbus_g_proxy_connect_signal(remote_object, "finish", G_CALLBACK(disnix_finish_signal_handler), &pid, NULL);
    dbus_g_proxy_connect_signal(remote_object, "success", G_CALLBACK(disnix_success_signal_handler), &pid, NULL);
    dbus_g_proxy_connect_signal(remote_object, "failure", G_CALLBACK(disnix_failure_signal_handler), &pid, NULL);

    /* Receive a PID for the job we want to execute */
    org_nixos_disnix_Disnix_get_job_id(remote_object, &pid, &error);
    g_printerr("Assigned PID: %d\n", pid);

    /* Execute operation */
    g_printerr("Executing operation.\n");
    
    switch(operation)
    {
	case OP_IMPORT:
	    if(derivation[0] == NULL)
	    {
		g_printerr("ERROR: A Nix store component has to be specified!\n");
		cleanup(derivation, arguments);
		return 1;
	    }
	    else
		org_nixos_disnix_Disnix_import(remote_object, pid, derivation[0], &error);
	    break;
	case OP_EXPORT:
	    org_nixos_disnix_Disnix_export(remote_object, pid, (const gchar**) derivation, &error);
	    break;
	case OP_PRINT_INVALID:
	    org_nixos_disnix_Disnix_print_invalid(remote_object, pid, (const gchar**) derivation, &error);
	    break;
	case OP_REALISE:
	    org_nixos_disnix_Disnix_realise(remote_object, pid, (const gchar**) derivation, &error);
	    break;
	case OP_SET:
	    org_nixos_disnix_Disnix_set(remote_object, pid, profile, derivation[0], &error);
	    break;
	case OP_QUERY_INSTALLED:
	    org_nixos_disnix_Disnix_query_installed(remote_object, pid, profile, &error);
	    break;
	case OP_QUERY_REQUISITES:
	    org_nixos_disnix_Disnix_query_requisites(remote_object, pid, (const gchar**) derivation, &error);
	    break;
	case OP_COLLECT_GARBAGE:
	    org_nixos_disnix_Disnix_collect_garbage(remote_object, pid, delete_old, &error);
	    break;
	case OP_ACTIVATE:
	    if(type == NULL)
	    {
		g_printerr("ERROR: A type must be specified!\n");
		cleanup(derivation, arguments);
		return 1;
	    }
	    else if(derivation[0] == NULL)
	    {
		g_printerr("ERROR: A Nix store component has to be specified!\n");
		cleanup(derivation, arguments);
		return 1;
	    }
	    else
		org_nixos_disnix_Disnix_activate(remote_object, pid, derivation[0], type, (const gchar**) arguments, &error);
	    break;
	case OP_DEACTIVATE:
	    if(type == NULL)
	    {
		g_printerr("ERROR: A type must be specified!\n");
		cleanup(derivation, arguments);
		return 1;
	    }
	    else if(derivation[0] == NULL)
	    {
		g_printerr("ERROR: A Nix store component has to be specified!\n");
		cleanup(derivation, arguments);
		return 1;
	    }
	    else
		org_nixos_disnix_Disnix_deactivate(remote_object, pid, derivation[0], type, (const gchar**) arguments, &error);
	    break;
	case OP_LOCK:
	    org_nixos_disnix_Disnix_lock(remote_object, pid, profile, &error);
	    break;
	case OP_UNLOCK:
	    org_nixos_disnix_Disnix_unlock(remote_object, pid, profile, &error);
	    break;
	case OP_SNAPSHOT:
	    if(type == NULL)
	    {
		g_printerr("ERROR: A type must be specified!\n");
		cleanup(derivation, arguments);
		return 1;
	    }
	    else if(derivation[0] == NULL)
	    {
		g_printerr("ERROR: A Nix store component has to be specified!\n");
		cleanup(derivation, arguments);
		return 1;
	    }
	    else
		org_nixos_disnix_Disnix_snapshot(remote_object, pid, derivation[0], type, (const gchar**) arguments, &error);
	    break;
	case OP_RESTORE:
	    if(type == NULL)
	    {
		g_printerr("ERROR: A type must be specified!\n");
		cleanup(derivation, arguments);
		return 1;
	    }
	    else if(derivation[0] == NULL)
	    {
		g_printerr("ERROR: A Nix store component has to be specified!\n");
		cleanup(derivation, arguments);
		return 1;
	    }
	    else
		org_nixos_disnix_Disnix_restore(remote_object, pid, derivation[0], type, (const gchar**) arguments, &error);
	    break;
	case OP_QUERY_ALL_SNAPSHOTS:
	    org_nixos_disnix_Disnix_query_all_snapshots(remote_object, pid, container, component, &error);
	    break;
	case OP_QUERY_LATEST_SNAPSHOT:
	    org_nixos_disnix_Disnix_query_latest_snapshot(remote_object, pid, container, component, &error);
	    break;
	case OP_PRINT_MISSING_SNAPSHOTS:
	    if(derivation[0] == NULL)
	    {
		g_printerr("ERROR: A Dysnomia snapshot has to be specified!\n");
		cleanup(derivation, arguments);
		return 1;
	    }
	    else
		org_nixos_disnix_Disnix_print_missing_snapshots(remote_object, pid, (const gchar**) derivation, &error);
	    break;
	case OP_IMPORT_SNAPSHOTS:
	    if(derivation[0] == NULL)
	    {
		g_printerr("ERROR: A Dysnomia snapshot has to be specified!\n");
		cleanup(derivation, arguments);
		return 1;
	    }
	    else
		org_nixos_disnix_Disnix_import_snapshots(remote_object, pid, container, component, (const gchar**) derivation, &error);
	    break;
	case OP_RESOLVE_SNAPSHOTS:
	    if(derivation[0] == NULL)
	    {
		g_printerr("ERROR: A Dysnomia snapshot has to be specified!\n");
		cleanup(derivation, arguments);
		return 1;
	    }
	    else
		org_nixos_disnix_Disnix_resolve_snapshots(remote_object, pid, (const gchar**) derivation, &error);
	    break;
	case OP_NONE:
	    g_printerr("ERROR: No operation specified!\n");
	    cleanup(derivation, arguments);
	    return 1;
    }

    if(error != NULL) 
    {
        g_printerr("Error while executing the operation! Reason: %s\n", error->message);
        cleanup(derivation, arguments);
        return 1;
    }
    
    /* Run loop and wait for signals */
    g_main_loop_run(mainloop);
    
    /* Operation is finished */
    cleanup(derivation, arguments);
    
    return EXIT_FAILURE;
}
