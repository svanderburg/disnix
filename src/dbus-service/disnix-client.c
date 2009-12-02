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

#include <stdio.h>
#include <stdlib.h>
#define _GNU_SOURCE
#include <getopt.h>
#include <glib.h>
#include <dbus/dbus-glib.h>
#include "disnix-client.h"
#include "disnix-marshal.h"

typedef enum
{
    OP_NONE,
    OP_IMPORT,
    OP_EXPORT,
    OP_PRINT_INVALID,
    OP_REALISE,
    OP_SET,
    OP_QUERY_INSTALLED,
    OP_QUERY_REQUISITES,
    OP_COLLECT_GARBAGE,
    OP_ACTIVATE,
    OP_DEACTIVATE,
    OP_LOCK,
    OP_UNLOCK,
}
Operation;

static void print_usage()
{
    /* Print the usage */
    printf("Usage:\n");
    printf("disnix-client --import [--localfile|--remotefile] derivations\n");
    printf("disnix-client --export [--localfile|--remotefile] derivations\n");
    printf("disnix-client --print-invalid derivations\n");
    printf("disnix-client {-r|--realise} derivations\n");
    printf("disnix-client --set [{-p|--profile} name] derivation\n");
    printf("disnix-client {-q|--query-installed} [{-p|--profile} name]\n");
    printf("disnix-client --query-requisites derivations\n");
    printf("disnix-client --collect-garbage [{-d|--delete-old}]\n");
    printf("disnix-client --activate --type type --arguments arguments derivation\n");
    printf("disnix-client --deactivate --type type --arguments arguments derivation\n");    
    printf("disnix-client --lock\n");
    printf("disnix-client --unlock\n");
    printf("disnix-client {-h|--help}\n");
}

/* Signal handlers */

static void disnix_finish_signal_handler(DBusGProxy *proxy, const gchar *pid, gpointer user_data)
{
    g_printerr("Received finish signal from pid: %s\n", pid);
    exit(0);
}

static void disnix_success_signal_handler(DBusGProxy *proxy, const gchar *pid, const gchar **derivation, gpointer user_data)
{
    unsigned int i;
    g_printerr("Received success signal from pid: %s\n", pid);
    
    for(i = 0; i < g_strv_length(derivation); i++)
	g_print("%s", derivation[i]);
	
    exit(0);
}

static void disnix_failure_signal_handler(DBusGProxy *proxy, const gchar *pid, gpointer user_data)
{
    g_printerr("Received failure signal from pid: %s\n", pid);
    exit(0);
}

/**
 * Runs the client
 */
static int run_disnix_client(Operation operation, gchar **derivation, int session_bus, char *profile, int delete_old, gchar **arguments, char *type)
{
    /* The GObject representing a D-Bus connection. */
    DBusGConnection *bus;
    
    /* Proxy object representing the D-Bus service object. */
    DBusGProxy *remote_object;

    /* GMainLoop object */
    GMainLoop *mainloop;
    
    /* Captures the results of D-Bus operations */
    GError *error = NULL;    
    gint reply;
    
    /* Other declarations */
    gchar *pid;
    
    /* If no operation is specified we should quit */
    if(operation == OP_NONE)
    {
	g_printerr("No operation is specified!\n");
	return 1;
    }
    
    /* Initialize the GType/GObject system. */
    g_type_init();

    /* Create main loop */
    mainloop = g_main_loop_new(NULL, FALSE);
    if(mainloop == NULL)
    {
	g_printerr("Cannot create main loop.\n");
	g_strfreev(derivation);
	g_strfreev(arguments);
	return 1;
    }

    /* Connect to the session/system bus */
    
    if(session_bus)
    {
	g_printerr("Connecting to the session bus.\n");
	bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
    }
    else
    {
	g_printerr("Connecting to the system bus.\n");
	bus = dbus_g_bus_get (DBUS_BUS_SYSTEM, &error);
    }
    
    if(!bus)
    {
        g_printerr("Cannot connect to session/system bus! Reason: %s\n", error->message);
	g_strfreev(derivation);
	g_strfreev(arguments);
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
	g_strfreev(derivation);
	g_strfreev(arguments);
        return 1;
    }

    /* Register marshaller for the return values of the success signal */
    dbus_g_object_register_marshaller(marshal_VOID__STRING_BOXED, G_TYPE_NONE, G_TYPE_STRING, G_TYPE_STRV, G_TYPE_INVALID);
    
    /* Register the signatures for the signal handlers */

    g_printerr("Add the argument signatures for the signal handler\n");
    dbus_g_proxy_add_signal(remote_object, "finish", G_TYPE_STRING, G_TYPE_INVALID);
    dbus_g_proxy_add_signal(remote_object, "success", G_TYPE_STRING, G_TYPE_STRV, G_TYPE_INVALID);
    dbus_g_proxy_add_signal(remote_object, "failure", G_TYPE_STRING, G_TYPE_INVALID);

    g_printerr("Register D-Bus signal handlers\n");
    dbus_g_proxy_connect_signal(remote_object, "finish", G_CALLBACK(disnix_finish_signal_handler), NULL, NULL);
    dbus_g_proxy_connect_signal(remote_object, "success", G_CALLBACK(disnix_success_signal_handler), NULL, NULL);
    dbus_g_proxy_connect_signal(remote_object, "failure", G_CALLBACK(disnix_failure_signal_handler), NULL, NULL);

    /* Execute operation */
    g_printerr("Executing operation.\n");
    
    switch(operation)
    {
	case OP_IMPORT:
	    org_nixos_disnix_Disnix_import(remote_object, derivation, &pid, &error);	        
	    break;
	case OP_EXPORT:
	    org_nixos_disnix_Disnix_export(remote_object, derivation, &pid, &error);
	    break;
	case OP_PRINT_INVALID:
	    org_nixos_disnix_Disnix_print_invalid(remote_object, derivation, &pid, &error);
	    break;
	case OP_REALISE:
	    org_nixos_disnix_Disnix_realise(remote_object, derivation, &pid, &error);
	    break;
	case OP_SET:
	    org_nixos_disnix_Disnix_set(remote_object, profile, derivation[0], &pid, &error);
	    break;
	case OP_QUERY_INSTALLED:
	    org_nixos_disnix_Disnix_query_installed(remote_object, profile, &pid, &error);
	    break;
	case OP_QUERY_REQUISITES:
	    org_nixos_disnix_Disnix_query_requisites(remote_object, derivation, &pid, &error);
	    break;
	case OP_COLLECT_GARBAGE:
	    org_nixos_disnix_Disnix_collect_garbage(remote_object, delete_old, &pid, &error);
	    break;
	case OP_ACTIVATE:
	    org_nixos_disnix_Disnix_activate(remote_object, derivation[0], type, arguments, &pid, &error);
	    break;
	case OP_DEACTIVATE:
	    org_nixos_disnix_Disnix_deactivate(remote_object, derivation[0], type, arguments, &pid, &error);
	    break;
	case OP_LOCK:
	    org_nixos_disnix_Disnix_lock(remote_object, &pid, &error);
	    break;
	case OP_UNLOCK:
	    org_nixos_disnix_Disnix_unlock(remote_object, &pid, &error);
	    break;
	case OP_NONE:
	    g_printerr("ERROR: No operation specified!\n");	    
	    print_usage();
	    g_strfreev(derivation);
	    g_strfreev(arguments);
	    return 1;
    }

    if (error != NULL) 
    {
        g_printerr("Error while executing the operation! Reason: %s\n", error->message);
	g_strfreev(derivation);
	g_strfreev(arguments);
	return 1;
    }
    
    /* Run loop and wait for signals */
    g_main_loop_run(mainloop);
    
    /* Operation is finished */
    g_strfreev(derivation);
    g_strfreev(arguments);
    return EXIT_FAILURE;
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
	{"import", no_argument, 0, 'I'},
	{"export", no_argument, 0, 'E'},
	{"print-invalid", no_argument, 0, 'P'},
	{"realise", no_argument, 0, 'r'},
	{"set", no_argument, 0, 'S'},
	{"query-installed", no_argument, 0, 'q'},
	{"query-requisites", no_argument, 0, 'Q'},
	{"collect-garbage", no_argument, 0, 'C'},
	{"activate", no_argument, 0, 'A'},
	{"deactivate", no_argument, 0, 'D'},
	{"lock", no_argument, 0, 'L'},
	{"unlock", no_argument, 0, 'U'},
	{"help", no_argument, 0, 'h'},
	{"target", required_argument, 0, 't'},
	{"localfile", no_argument, 0, 'l'},
	{"remotefile", no_argument, 0, 'R'},
	{"profile", required_argument, 0, 'p'},
	{"delete-old", no_argument, 0, 'd'},
	{"type", required_argument, 0, 'T'},
	{"arguments", required_argument, 0, 'a'},
	{"session-bus", no_argument, 0, 'b'},
	{0, 0, 0, 0}
    };

    /* Option value declarations */
    Operation operation = OP_NONE;
    char *target, *profile = "default", *type;
    gchar **derivation = NULL, **arguments = NULL;
    unsigned int derivation_size = 0;
    int localfile = FALSE, remotefile = TRUE;
    int delete_old = FALSE, session_bus = FALSE;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "rqt:p:dh", long_options, &option_index)) != -1)
    {
	switch(c)
	{	    
	    case 'I':
		operation = OP_IMPORT;
		break;
	    case 'E':
		operation = OP_EXPORT;		
		break;
	    case 'P':
		operation = OP_PRINT_INVALID;		
		break;
	    case 'r':
		operation = OP_REALISE;
		break;
	    case 'S':
		operation = OP_SET;
		break;
	    case 'q':
		operation = OP_QUERY_INSTALLED;
		break;
	    case 'Q':
		operation = OP_QUERY_REQUISITES;
		break;
	    case 'C':
		operation = OP_COLLECT_GARBAGE;
		break;
	    case 'A':
		operation = OP_ACTIVATE;
		break;
	    case 'D':
		operation = OP_DEACTIVATE;
		break;
	    case 'L':
		operation = OP_LOCK;
		break;
	    case 'U':
		operation = OP_UNLOCK;
		break;
	    case 't':
		target = optarg;
		break;
	    case 'l':
		localfile = TRUE;
		remotefile = FALSE;
		break;
	    case 'R':
		localfile = FALSE;
		remotefile = TRUE;
		break;
	    case 'p':
		profile = optarg;
		break;
	    case 'd':
		delete_old = TRUE;
		break;
	    case 'T':
		type = optarg;
		break;
	    case 'a':
		arguments = g_strsplit(optarg, " ", 0);
		break;
	    case 'b':
		session_bus = TRUE;
		break;
	    case 'h':
		print_usage();
		return 0;
	}
    }
    
    /* Validate non-options */
    while(optind < argc)
    {
	derivation = g_realloc(derivation, (derivation_size + 1) * sizeof(gchar*));
	derivation[derivation_size] = g_strdup(argv[optind]);
	derivation_size++;	
	optind++;
    }
    
    derivation = g_realloc(derivation, (derivation_size + 1) * sizeof(gchar*));
    derivation[derivation_size] = NULL;
    
    /* Execute Disnix client */
    return run_disnix_client(operation, derivation, session_bus, profile, delete_old, arguments, type);
}
