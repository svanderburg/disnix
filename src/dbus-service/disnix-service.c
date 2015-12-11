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

#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <dbus/dbus-glib.h>
#include <glib.h>

/* Server settings variables */

/** Path to the temp directory */
char *tmpdir;

/** Path to the log directory */
char *logdir;

/** Stores the original signal action for the SIGCHLD signal */
struct sigaction oldact;

/* Provides each job a unique job id */
int job_counter;

/* Disnix instance definition */
#include "disnix-instance.h"

/* GType convienence macros */
#include "disnix-gtype-def.h"

/*
 * Forward declarations of D-Bus methods.
 * Since the stub generator will reference the functions from a call
 * table, the functions must be declared before the stub is included.
 */
#include "methods.h"

/* Include the generated server code */
#include "disnix-service.h"

/* Include the signal definitions */
#include "signals.h"

/* Utility macro to define the value_object GType structure. */
G_DEFINE_TYPE(DisnixObject, disnix_object, G_TYPE_OBJECT)

/**
 * Initializes a Disnix object instance
 *
 * @param obj Disnix object instance to initialize
 */
static void disnix_object_init(DisnixObject *obj)
{
    g_assert(obj != NULL);
    obj->pid = 0;
}

/**
 * Initializes the Disnix class
 *
 * @param klass Class to initialize
 */
static void disnix_object_class_init(DisnixObjectClass *klass)
{
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
						    G_TYPE_INT /* Parameter types */);
						    
    klass->signals[E_SUCCESS_SIGNAL] = g_signal_new ("success",
                                                    G_OBJECT_CLASS_TYPE (klass),
						    G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
						    0,
						    NULL, NULL,
						    g_cclosure_marshal_VOID__STRING,
						    G_TYPE_NONE,
						    2,
						    G_TYPE_INT, G_TYPE_STRV);

    klass->signals[E_FAILURE_SIGNAL] = g_signal_new ("failure",
                                                    G_OBJECT_CLASS_TYPE (klass),
						    G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
						    0,
						    NULL, NULL,
						    g_cclosure_marshal_VOID__STRING,
						    G_TYPE_NONE,
						    1,
						    G_TYPE_INT);
    
    /* Bind this GType into the Glib/D-Bus wrappers */
    dbus_g_object_type_install_info (DISNIX_TYPE_OBJECT, &dbus_glib_disnix_object_info);
}

/**
 * Handles a SIGCHLD signal.
 *
 * @param signum Singnal number
 */
 
static void handle_sigchild(int signum)
{
    int status;
    wait(&status);
}

static int filter_irrelevant(const struct dirent *entity)
{
    if(strcmp(entity->d_name, ".") == 0 || strcmp(entity->d_name, "..") == 0)
        return FALSE;
    else
        return TRUE;
}

static int numbersort(const struct dirent **a, const struct dirent **b)
{
    int left = atoi((*a)->d_name);
    int right = atoi((*b)->d_name);
    
    if(left < right)
        return -1;
    else if(left > right)
        return 1;
    else
        return 0;
}

int start_disnix_service(int session_bus, char *log_path)
{
    /* The D-Bus connection object provided by dbus_glib */
    DBusGConnection *bus;
    
    /* 
     * Representation of the D-Bus value object locally.
     * (this object acts as a proxy for method calls and signal delivery)
     */
    DBusGProxy *proxy;
    
    /* Holds one instance of DisnixObject that will serve all the requests. */
    DisnixObject *object;
    
    /* GLib mainloop that keeps the server running */
    GMainLoop *mainloop;
    
    /* Variables used to store error information */
    GError *error = NULL;
    guint result;

    /* Specifies the new action for a SIGCHLD signal */
    struct sigaction act;
    
    /* Contains information about the amount of logfiles in the log directory */
    struct dirent **namelist;
    int num_of_entries;
    
    /* Determine the temp directory */
    tmpdir = getenv("TMPDIR");
    
    if(tmpdir == NULL)
	tmpdir = "/tmp";
    
    /* Determine the log directory */
    logdir = log_path;
    
    /* Create a new SIGCHLD handler which discards the result of the child process */
    act.sa_handler = handle_sigchild;
    act.sa_flags = 0;
    sigaction(SIGCHLD, &act, &oldact);
    
    /* Create a GMainloop with initial state of 'not running' (FALSE) */
    mainloop = g_main_loop_new(NULL, FALSE);
    if(mainloop == NULL)
    {
	g_printerr("ERROR: Failed to create the mainloop!\n");
	return 1;
    }
    
    /* Connect to the system/session bus */
        
    if(session_bus)
    {
	g_print("Connecting to the session bus\n");
	bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
    }
    else
    {
	g_print("Connecting to the system bus\n");
	bus = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);
    }
	
    if(error != NULL)
    {
	g_printerr("ERROR: Cannot connect to the system/session bus! Reason: %s\n", error->message);
	g_error_free(error);
	return 1;
    }
    
    /* Create a GLib proxy object */
    
    g_print("Creating a GLib proxy object\n");    
    proxy = dbus_g_proxy_new_for_name (bus, "org.freedesktop.DBus", /* Name */
 					    "/org/freedesktop/DBus", /* Object path */
					    "org.freedesktop.DBus"); /* Interface */
    if(proxy == NULL)
    {
	g_printerr("Cannot create a proxy object!\n");
	return 1;
    }
    
    /* Attempt to register the well-known name of the server */
    
    g_print("Registering the org.nixos.disnix.Disnix as the well-known name\n");
    
    if (!dbus_g_proxy_call (proxy, "RequestName", &error,
			    G_TYPE_STRING, "org.nixos.disnix.Disnix",
			    G_TYPE_UINT, 0,
			    G_TYPE_INVALID,
			    G_TYPE_UINT, &result,
			    G_TYPE_INVALID))
    {
        g_printerr("D-Bus.RequestName RPC failed! Reason: %s\n", error->message);
        return 1;
    }

    /* Check the result of the registration */
    g_print("RequestName returned: %d\n", result);
    
    if(result != 1)
    {
        g_printerr("Failed to get the primary well-known name!\n");
        return 1;
    }

    /* Create one single instance */
    g_print("Creating a single Disnix instance\n");
     
    object = g_object_new(DISNIX_TYPE_OBJECT, NULL);
    if(!object)
    {
        g_printerr("Failed to create one Disnix instance.\n");
        return 1;
    }
    
    /* Register the instance to D-Bus */
    g_print("Registering the Disnix instance to D-Bus\n");
    dbus_g_connection_register_g_object (bus, "/org/nixos/disnix/Disnix", G_OBJECT(object));
    
    /* Figure out what the next job id number is */
    num_of_entries = scandir(logdir, &namelist, 0, numbersort);
    
    if(num_of_entries <= 0)
       job_counter = 0;
    else
    {
       char *filename = namelist[num_of_entries - 1]->d_name;
       job_counter = atoi(filename);
       job_counter++;
       free(namelist[num_of_entries - 1]);
       free(namelist);
    }
    
    /* Starting the main loop */
    g_print("The Disnix is service is running!\n");
    g_main_loop_run(mainloop);
    
    /* The main loop should not be stopped, but if it does return the exit failure status */
    return EXIT_FAILURE;
}
