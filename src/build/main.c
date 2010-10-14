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
#include <derivationmapping.h>
#include <defaultoptions.h>
#include <client-interface.h>

#define BUFFER_SIZE 4096

static void print_usage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "disnix-build [--interface interface] distributed_derivation\n");
    fprintf(stderr, "disnix-build {-h | --help}\n");
}

static void delete_result_array(GArray *result_array)
{
    unsigned int i;
    
    for(i = 0; i < result_array->len; i++)
    {
	gchar *result = g_array_index(result_array, gchar*, i);
	g_free(result);
    }
    
    g_array_free(result_array, TRUE);
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

    interface = check_interface_option(interface);
    
    if(optind >= argc)
    {
	fprintf(stderr, "ERROR: No manifest specified!\n");
	return 1;
    }
    else
    {		
	/* Generate a distribution array from the manifest file */
	GArray *derivation_array = create_derivation_array(argv[optind]);
	
	if(derivation_array == NULL)
	    return -1;
	else
	{
	    unsigned int i, running_processes = 0;
	    int exit_status = 0;
	    int status;
	    
	    GArray *output_array;
	    GArray *result_array;
	
	    /* Iterate over the derivation array and distribute the store derivation closures to the target machines */
	    for(i = 0; i < derivation_array->len; i++)
	    {
		DerivationItem *item = g_array_index(derivation_array, DerivationItem*, i);
		int status;
	    
		fprintf(stderr, "Distributing intra-dependency closure of derivation: %s to target: %s\n", item->derivation, item->target);
		
		status = wait_to_finish(exec_copy_closure_to(interface, item->target, item->derivation));
		
		/* On error stop the build process */
		if(status != 0)
		{
		    delete_derivation_array(derivation_array);
		    return status;
		}
	    }

	    /* Iterate over the derivation array and invoke processes that realise the derivations remotely */
	
	    output_array = g_array_new(FALSE, FALSE, sizeof(int));    
	
	    for(i = 0; i < derivation_array->len; i++)
	    {
		int pipefd[2];
		int status;
		DerivationItem *item = g_array_index(derivation_array, DerivationItem*, i);
		
		status = exec_realise(interface, item->target, item->derivation, pipefd);
		
		if(status == -1)
		{
		    fprintf(stderr, "Error with forking realise process!\n");
		    exit_status = -1;
		}
		else
		{
		    g_array_append_val(output_array, pipefd[0]); /* Append read file descriptor to array */
		    running_processes++;
		}
	    }
	    
	    result_array = g_array_new(FALSE, FALSE, sizeof(gchar*));
		
	    /* Capture the output (Nix store components) of every realise process */
	    
	    for(i = 0; i < output_array->len; i++)
	    {
		char line[BUFFER_SIZE];
		int pipefd = g_array_index(output_array, int, i);
		ssize_t line_size;
		    
		while((line_size = read(pipefd, line, BUFFER_SIZE - 1)) > 0)
		{
		    line[line_size] = '\0';
		    puts(line);
		    
		    if(g_strcmp0(line, "\n") != 0)
		    {
		    	gchar *result;
		    	result = g_strdup(line);
		        g_array_append_val(result_array, result);
		    }
	        }	    		    
		
	        close(pipefd);
	    }

	    /* Check statusses of the running processes */
	    for(i = 0; i < running_processes; i++)
	    {
		/* Wait until a realise process is finished */
		wait(&status);
	    
		/* If one of the processes fail, change the exit status */
		if(WEXITSTATUS(status) != 0)
	    	    exit_status = WEXITSTATUS(status);		    
	    }
	    
	    if(exit_status == 0)
	    {	    	
		/* Retrieve back the realised closures and import them into the Nix store of the host */
    
		for(i = 0; i < derivation_array->len; i++)
		{
		    gchar *result;
		    DerivationItem *item;
		    int status;
	
		    result = g_array_index(result_array, gchar*, i);
		    item = g_array_index(derivation_array, DerivationItem*, i);
	
	    	    fprintf(stderr, "Copying result: %s from: %s\n", result, item->target);
		    
		    status = wait_to_finish(exec_copy_closure_from(interface, item->target, result));
		    
		    /* On error stop build process */
		    if(status != 0)
		    {
		        g_array_free(output_array, TRUE);
		        delete_result_array(result_array);
    		        delete_derivation_array(derivation_array);
		        return status;
		    }
		}
	    }
	
	    /* Cleanup */
	    g_array_free(output_array, TRUE);
	    delete_result_array(result_array);
	    delete_derivation_array(derivation_array);
	    
	    return exit_status;
	}
    }
}
