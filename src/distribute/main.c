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
#include <sys/types.h>
#include <sys/wait.h>
#define _GNU_SOURCE
#include <getopt.h>
#include <glib.h>
#include "distributionmapping.h"

static void print_usage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "disnix-distribute [--interface] interface] distribution_export\n");
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
    gchar *interface_arg = NULL;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "h", long_options, &option_index)) != -1)
    {
	switch(c)
	{
	    case 'i':
		interface_arg = g_strconcat(" --interface ", optarg, NULL);
		break;
	    case 'h':
		print_usage();
		return 0;
	}
    }

    /* Validate options */
    if(interface_arg == NULL)
    {
	char *interface_env = getenv("DISNIX_CLIENT_INTERFACE");
	
	if(interface_env == NULL)
	    interface_arg = g_strdup("");
	else
	    interface_arg = g_strconcat(" --interface ", interface_env, NULL);
    }


    if(optind >= argc)
    {
	fprintf(stderr, "ERROR: No distribution export specified!\n");
	g_free(interface_arg);
	return 1;
    }
    else
    {
	int exit_status = 0;
	unsigned int i;
	GArray *distribution_array;
	
	/* Generate a distribution array from the distribution export file */
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
		gchar *command;
	    
		fprintf(stderr, "Distributing intra-dependency closure of profile: %s to target: %s\n", item->profile, item->target);
		command = g_strconcat("disnix-copy-closure --to --target ", item->target, interface_arg, " ", item->profile, NULL);
		status = system(command);
	    
		/* Cleanups */
		g_free(command);
	    
		/* On error stop the distribute process */
		if(status == -1)
		{
		    exit_status = -1;
		    break;
		}
		else if(WEXITSTATUS(status) != 0)
		{
		    exit_status = WEXITSTATUS(status);
		    break;
		}
	    }
	    
	    delete_distribution_array(distribution_array);
	}
	
	/* Clean up */
	g_free(interface_arg);
	xmlCleanupParser();
    
	/* Return the exit status, which is 0 if everything succeeds */
	return exit_status;
    }
}
