/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008  Sander van der Burg
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
#include <stdio.h>
#include <unistd.h>
#define _GNU_SOURCE
#include <getopt.h>
#include "disnix-client.h"
#include "disnix-marshal.h"

typedef enum
{
    OP_NONE,
    OP_INSTALL,
    OP_UPGRADE,
    OP_UNINSTALL,
    OP_INSTANTIATE,
    OP_REALISE,
    OP_IMPORT,
    OP_PRINT_INVALID_PATHS,
    OP_COLLECT_GARBAGE,
    OP_ACTIVATE,
    OP_DEACTIVATE
}
Operation;

static void print_usage()
{
    g_print("Usage:\n");
    g_print("disnix-client {-i | --install} [-f file] [-A | --attr] args\n");
    g_print("disnix-client {-u | --upgrade} derivation\n");
    g_print("disnix-client {-e | --uninstall} derivation\n");
    g_print("disnix-client --instantiate [-A attributepath | --attr attributepath] filename\n");
    g_print("disnix-client {-r | --realise} pathname\n");
    g_print("disnix-client --import filename\n");
    g_print("disnix-client --print-invalid-paths paths\n");
    g_print("disnix-client --collect-garbage [-d | --delete-old]\n");
    g_print("disnix-client --type type --activate path\n");
    g_print("disnix-client --type type --deactivate path\n");
    g_print("disnix-client {-h | --help}\n");
}

/* Signal handler for status signal */

static void disnix_finish_signal_handler(DBusGProxy *proxy, const gchar *pid, gpointer user_data)
{
    g_printerr("Received finish signal from pid: %s\n", pid);
    _exit(0);
}

static void disnix_success_signal_handler(DBusGProxy *proxy, const gchar *pid, const gchar *path, gpointer user_data)
{
    g_printerr("Received success signal from pid: %s, results path: %s\n", pid, path);
    g_print(path);
    _exit(0);
}

static void disnix_failure_signal_handler(DBusGProxy *proxy, const gchar *pid, gpointer user_data)
{
    g_printerr("Received failure signal from pid: %s\n", pid);
    _exit(0);
}

int main(int argc, char **argv)
{
    /* The GObject representing a D-Bus connection. */
    DBusGConnection *bus;
    /* Proxy object representing the D-Bus service object. */
    DBusGProxy *remote_object;
    /* GMainLoop object */
    GMainLoop *mainloop;
    GError *error = NULL;    
    gint reply;
    int option_index = 0, c;
    struct option long_options[] =
    {
	{"install", no_argument, 0, 'i'},
	{"upgrade", required_argument, 0, 'u'},
	{"uninstall", required_argument, 0, 'e'},
	{"instantiate", no_argument, 0, 'I'},
	{"realise", required_argument, 0, 'r'},
	{"import", required_argument, 0, 'M'},
	{"print-invalid-paths", no_argument, 0, 'P'},
	{"collect-garbage", no_argument, 0, 'C'},
	{"activate", required_argument, 0, 'Q'},
	{"deactivate", required_argument, 0, 'W'},
	{"attr", required_argument, 0, 'A'},
	{"delete-old", no_argument, 0, 'd'},
	{"type", required_argument, 0, 't'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
    };
    
    /* Initialize the GType/GObject system. */
    g_type_init ();

    /* Create main loop */
    mainloop = g_main_loop_new (NULL, FALSE);
    if(mainloop == NULL)
    {
	g_printerr ("Cannot create main loop.\n");
	return 1;
    }

    /*g_print (" : Connecting to Session D-Bus.\n");
    bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);*/
    
    g_printerr (" : Connecting to System D-Bus.\n");
    bus = dbus_g_bus_get (DBUS_BUS_SYSTEM, &error);
    
    if (!bus) {
        /* Print error and terminate. */
        g_printerr (" : Couldn't connect to session bus, %s\n", error->message);
        return 1;
    }

    /* Create the proxy object that will be used to access the object */
    g_printerr (" : Creating a Glib proxy object.\n");
    remote_object = dbus_g_proxy_new_for_name (bus,
					       "org.nixos.disnix.Disnix",           /* name */
					       "/org/nixos/disnix/Disnix",          /* obj path */
					       "org.nixos.disnix.Disnix");          /* interface */

    if(remote_object == NULL) {
        /* Print error and terminate. */
        g_printerr (" : Couldn't create the proxy object, %s\n", error->message);
        return 1;
    }
    
    /* Register the signatures for the signal handlers */

    dbus_g_object_register_marshaller(marshal_VOID__STRING_STRING, G_TYPE_NONE, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INVALID);    
    g_printerr (" : Add the argument signatures for the signal handler\n");
    dbus_g_proxy_add_signal(remote_object, "finish", G_TYPE_STRING, G_TYPE_INVALID);
    dbus_g_proxy_add_signal(remote_object, "success", G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INVALID);
    dbus_g_proxy_add_signal(remote_object, "failure", G_TYPE_STRING, G_TYPE_INVALID);
    g_printerr (" : Register D-Bus signal handlers\n");
    dbus_g_proxy_connect_signal(remote_object, "finish", G_CALLBACK(disnix_finish_signal_handler), NULL, NULL);
    dbus_g_proxy_connect_signal(remote_object, "success", G_CALLBACK(disnix_success_signal_handler), NULL, NULL);
    dbus_g_proxy_connect_signal(remote_object, "failure", G_CALLBACK(disnix_failure_signal_handler), NULL, NULL);

    /* Do a call */
    
    g_printerr (" : Call instantiate\n");
    gchar *pid = NULL, *args, *derivation, *attr = NULL, *filename, *pathname, *file = NULL, *type;
    gchar *paths, *oldPaths;
    gboolean isAttr, delete_old = FALSE;
    
    Operation operation = OP_NONE;
        
    while((c = getopt_long(argc, argv, "iu:e:A:r:f:t:dh", long_options, &option_index)) != -1)
    {
	switch(c)
	{
	    case 'i':
		operation = OP_INSTALL;
		break;
	
	    case 'u':
		operation = OP_UPGRADE;
		derivation = optarg;
		break;
	
	    case 'e':
		operation = OP_UNINSTALL;
		derivation = optarg;
		break;
	    
	    case 'A':
		attr = optarg;
		break;
		
	    case 'I':
		operation = OP_INSTANTIATE;
		break;
	
	    case 'r':
		operation = OP_REALISE;
		pathname = optarg;
		break;
	    
	    case 'M':
		operation = OP_IMPORT;
		filename = optarg;
		break;
	
	    case 'P':
		operation = OP_PRINT_INVALID_PATHS;
		break;
	
	    case 'C':
		operation = OP_COLLECT_GARBAGE;
		break;
	
	    case 'Q':
	        operation = OP_ACTIVATE;
		pathname = optarg;
		break;

	    case 'W':
	        operation = OP_DEACTIVATE;
		pathname = optarg;
		break;
		
	    case 'f':
		file = optarg;
		break;
	
	    case 'd':
		delete_old = TRUE;
		break;
	
	    case 't':
		type = optarg;
		break;
		
	    case 'h':
		print_usage();
		_exit(0);    
		break;
	    
	    default:
		print_usage();
		_exit(1);
	}
    }
    
    /* Execute operation */
    
    switch(operation)
    {
	case OP_INSTALL:
	    isAttr = (attr != NULL);
	
	    if(isAttr)
		args = attr;
	    else
	    {
		gchar *oldArgs;		
		args = g_strdup("");
		
		while(optind < argc)
		{
		    oldArgs = args;
		    args = g_strconcat(oldArgs, " ", argv[optind++], NULL);
		    g_free(oldArgs);
		}
	    }
	    
	    org_nixos_disnix_Disnix_install(remote_object, file, args, isAttr, &pid, &error);
	    
	    if(!isAttr)
		g_free(args);
		
	    break;
    
	case OP_UPGRADE:
	    org_nixos_disnix_Disnix_upgrade(remote_object, derivation, &pid, &error);
	    break;
    
	case OP_UNINSTALL:
	    org_nixos_disnix_Disnix_uninstall(remote_object, derivation, &pid, &error);
	    break;
    
	case OP_INSTANTIATE:
	    if(optind < argc)
	    {
		filename = argv[optind];
		org_nixos_disnix_Disnix_instantiate(remote_object, filename, attr, &pid, &error);
	    }
	    else
	    {
		g_printerr("You must specify a filename!\n");
		_exit(1);
	    }
	    break;
	
	case OP_REALISE:
	    org_nixos_disnix_Disnix_realise(remote_object, pathname, &pid, &error);
	    break;
	
	case OP_IMPORT:
	    org_nixos_disnix_Disnix_realise(remote_object, filename, &pid, &error);
	    break;
	
	case OP_PRINT_INVALID_PATHS:	    
	    paths = g_strdup("");
	    
	    while(optind < argc)
	    {
		oldPaths = paths;
		paths = g_strconcat(oldPaths, " ", argv[optind++], NULL);
		g_free(oldPaths);
	    }
	
	    org_nixos_disnix_Disnix_print_invalid_paths(remote_object, paths, &pid, &error);
	    g_free(paths);
	    break;
	
	case OP_COLLECT_GARBAGE:
	    org_nixos_disnix_Disnix_collect_garbage(remote_object, delete_old, &pid, &error);
	    break;
	  
	case OP_ACTIVATE:
	    org_nixos_disnix_Disnix_activate(remote_object, pathname, type, &pid, &error);
	    break;

	case OP_DEACTIVATE:
	    org_nixos_disnix_Disnix_deactivate(remote_object, pathname, type, &pid, &error);
	    break;
	    
	default:
	    g_printerr("You need to specify an operation!\n");
	    print_usage();
	    _exit(1);
    }
    
    if (error  != NULL) 
        g_printerr (" : fail, %s.\n", error->message);
    /*else 
        g_print (" : success, got reply %d.\n", reply);*/

    /* Run loop and wait for signal */
    g_main_loop_run(mainloop);
    
    return 0;
}
