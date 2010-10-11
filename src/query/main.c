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
#include <unistd.h>
#include <getopt.h>
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <glib.h>
#include <infrastructure.h>
#include <defaultoptions.h>

static void print_usage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "disnix-query [--interface interface] [--target-property targetProperty] [{-p | --profile} profile] infrastructure_expr\n");
    fprintf(stderr, "disnix-query {-h | --help}\n");
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
	{"interface", required_argument, 0, 'i'},
	{"target-property", required_argument, 0, 't'},
	{"profile", required_argument, 0, 'p'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
    };
    char *interface = NULL;
    char *target_property = NULL;
    char *profile = NULL;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "p:h", long_options, &option_index)) != -1)
    {
	switch(c)
	{
	    case 'i':
		interface = optarg;
		break;
	    case 't':
		target_property = optarg;
		break;
	    case 'p':
		profile = optarg;
		break;
	    case 'h':
		print_usage();
		return 0;
	}
    }

    /* Validate options */
    
    interface = check_interface_option(interface);
    target_property = check_target_property_option(target_property);
    profile = check_profile_option(profile);
    
    if(optind >= argc)
    {
	fprintf(stderr, "A infrastructure Nix expression has to be specified!\n");
	return 1;
    }
    else
    {
	GArray *target_array = create_target_array(argv[optind], target_property);
	
	if(target_array != NULL)
	{
	    unsigned int i;
	    
	    for(i = 0; i < target_array->len; i++)
	    {
		gchar *target = g_array_index(target_array, gchar*, i);
		int status;
		
		printf("\nServices on: %s\n\n", target);
		
		status = fork();
		
		if(status == 0)
		{
		    char *args[] = {interface, "--target", target, "--profile", profile, "--query-installed", NULL};
		    execvp(interface, args);
		    fprintf(stderr, "Error with executing query process!\n");
		    _exit(1);
		}
		
		if(status == -1)
		{
		    fprintf(stderr, "Error with forking query process!\n");
		    delete_target_array(target_array);
		    return -1;
		}
		else
		{		
		    wait(&status);
				
		    if(WEXITSTATUS(status) != 0)
		    {
			delete_target_array(target_array);
	    		return WEXITSTATUS(status);
		    }
		}
	    }
	    
	    delete_target_array(target_array);
	}

	return 0;
    }
}
