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
#include <sys/types.h>
#include <sys/wait.h>
#define _GNU_SOURCE
#include <getopt.h>
#include <glib.h>
#include "distributionmapping.h"

static void print_usage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "disnix-distribute [--interface interface] manifest\n");
    fprintf(stderr, "disnix-distribute {-h | --help}\n");
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
	{"interface", required_argument, 0, 'i'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
    };
    char *interface = NULL;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "h", long_options, &option_index)) != -1)
    {
	switch(c)
	{
	    case 'i':
		interface = optarg;
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
	    interface = g_strdup("disnix-client");
	else
	    interface = interface_env;
    }


    if(optind >= argc)
    {
	fprintf(stderr, "ERROR: No manifest specified!\n");
	return 1;
    }
    else
    {
	int exit_status = 0;
	unsigned int i;
	GArray *distribution_array;
	
	/* Generate a distribution array from the manifest file */
	distribution_array = generate_distribution_array(argv[optind]);
	
	if(distribution_array == NULL)
	    exit_status = 1;
	else
	{
	    /* Iterate over the distribution array and distribute the profiles to the target machines */
	    for(i = 0; i < distribution_array->len; i++)
	    {
		DistributionItem *item = g_array_index(distribution_array, DistributionItem*, i);
		int status;
	    
		fprintf(stderr, "Distributing intra-dependency closure of profile: %s to target: %s\n", item->profile, item->target);
		
		status = fork();
		
		if(status == -1)
		{
		    exit_status = -1;
		    break;
		}
		else if(status == 0)
		{
		    char *args[] = {"disnix-copy-closure", "--to", "--target", item->target, "--interface", interface, item->profile, NULL};
		    execvp("disnix-copy-closure", args);
		    fprintf(stderr, "Error with executing copy closure process!\n");
		    _exit(1);
		}
		
	        wait(&status);
		
		/* On error stop the distribute process */
		if(WEXITSTATUS(status) != 0)
		{
		    exit_status = WEXITSTATUS(status);
		    break;
		}
	    }
	    
	    delete_distribution_array(distribution_array);
	}
	
	/* Clean up */
	xmlCleanupParser();
    
	/* Return the exit status, which is 0 if everything succeeds */
	return exit_status;
    }
}
