/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2010  Sander van der Burg
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
#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <glib.h>
#include "infrastructure.h"

static void print_usage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "disnix-collect-garbage [--interface interface] [--target-property targetProperty] [-d|--delete-old] infrastructure_expr\n");
    fprintf(stderr, "disnix-collect-garbage {-h | --help}\n");
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
	{"interface", required_argument, 0, 'i'},
	{"target-property", required_argument, 0, 't'},
	{"delete-old", no_argument, 0, 'd'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
    };
    char *interface = NULL;
    char *delete_old_arg = "";
    char *target_property = NULL;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "dh", long_options, &option_index)) != -1)
    {
	switch(c)
	{
	    case 'i':
		interface = optarg;
		break;
	    case 't':
		target_property = optarg;
		break;
	    case 'd':
	        delete_old_arg = "-d";
	        break;
	    case 'h':
		print_usage();
		return 0;
	}
    }
    
    /* Validate options */
    if(interface == NULL)
    {
	char *interface_env = getenv("DISNIX_CLIENT_INTERFACE");
	
	if(interface_env == NULL)
	    interface = "disnix-client";
	else
	    interface = interface_env;
    }

    if(target_property == NULL)
    {
	char *target_property_env = getenv("DISNIX_TARGET_PROPERTY");
	
	if(target_property_env == NULL)
	    target_property = "hostname";
	else
	    target_property = target_property_env;
    }

    if(optind >= argc)
    {
	fprintf(stderr, "An infrastructure Nix expression has to be specified!\n");
	return 1;
    }
    else
    {
	GArray *target_array = create_target_array(argv[optind], target_property);
	int exit_status = 0;
	
	if(target_array != NULL)
	{
	    unsigned int i, running_processes = 0;	    
	    int status;
	    
	    /* Spawn garbage collection processes */
	    for(i = 0; i < target_array->len; i++)
	    {
		gchar *target = g_array_index(target_array, gchar*, i);
		char *args[] = {interface, "--target", target, "--collect-garbage", delete_old_arg, NULL};
		int status;
		
		printf("Collecting garbage on: %s\n", target);
		
		status = fork();
		
		if(status == -1)
		{
		    fprintf(stderr, "Error forking garbage collection process!\n");
		    exit_status = -1;
		}
		else if(status == 0)
		{
		    execvp(interface, args);
		    fprintf(stderr, "Error starting garbage collection process on: %s\n", target);
		    _exit(1);
		}
		else
		    running_processes++;
	    }
	    	    
	    /* Check statusses of the running processes */	    
	    for(i = 0; i < running_processes; i++)
	    {
		/* Wait until a garbage collector processes is finished */
		wait(&status);
	    
		/* If one of the processes fail, change the exit status */
		if(WEXITSTATUS(status) != 0)
	    	    exit_status = WEXITSTATUS(status);
	    }
	    	
	    delete_target_array(target_array);
	}
	
	return exit_status;
    }
}
