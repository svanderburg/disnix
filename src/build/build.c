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

#include "build.h"
#include <derivationmapping.h>
#include <client-interface.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 4096

static int distribute_derivations(gchar *interface, GArray *derivation_array)
{
    int status = 0;
    unsigned int i;
    
    for(i = 0; i < derivation_array->len; i++)
    {	
	/* Retrieve derivation item from array */
	DerivationItem *item = g_array_index(derivation_array, DerivationItem*, i);
	    
	g_printerr("Distributing intra-dependency closure of derivation: %s to target: %s\n", item->derivation, item->target);
		
	/* Execute copy closure process */
	status = wait_to_finish(exec_copy_closure_to(interface, item->target, item->derivation));
		
	/* On error stop the distribution process */
	if(status != 0)	    
	    return status;
    }
    
    return status;
}

static int realise(gchar *interface, GArray *derivation_array, GArray *result_array)
{
    /* Declarations */
    
    unsigned int i;
    GArray *output_array = g_array_new(FALSE, FALSE, sizeof(int));    
    int exit_status = 0;
    int running_processes = 0;
    int status;
    
    /* Iterate over the derivation array and distribute the store derivation closures to the target machines */
    
    for(i = 0; i < derivation_array->len; i++)
    {
	int pipefd[2];	
	DerivationItem *item = g_array_index(derivation_array, DerivationItem*, i);
		
	status = exec_realise(interface, item->target, item->derivation, pipefd);
		
	if(status == -1)
	{
	    g_printerr("Error with forking realise process!\n");
	    exit_status = -1;
	}
	else
	{
	    g_array_append_val(output_array, pipefd[0]); /* Append read file descriptor to array */
	    running_processes++;
	}
    }
    	
    /* Capture the output (Nix store components) of every realise process */

    result_array = g_array_new(FALSE, FALSE, sizeof(gchar*));
    
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
    
    /* Cleanup */
    g_array_free(output_array, TRUE);
    
    /* Return the exit status, which is 0 if everything succeeds */
    return exit_status;
}

static int retrieve_results(gchar *interface, GArray *derivation_array, GArray *result_array)
{
    unsigned int i;
    int status = 0;
    
    /* Iterate over the derivation array and copy the build results back */
    for(i = 0; i < derivation_array->len; i++)
    {
	gchar *result;
	DerivationItem *item;
	
	result = g_array_index(result_array, gchar*, i);
	item = g_array_index(derivation_array, DerivationItem*, i);
	
	g_printerr("Copying result: %s from: %s\n", result, item->target);
		    
	status = wait_to_finish(exec_copy_closure_from(interface, item->target, result));
		    
	/* On error stop build process */
	if(status != 0)
	    return status;
    }
    
    return status;
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

int build(gchar *interface, gchar *distributed_derivation_file)
{
    GArray *derivation_array = create_derivation_array(distributed_derivation_file);        
    int exit_status = 0;
    
    /* Distribute derivations to target machines */
    if((exit_status = distribute_derivations(interface, derivation_array)) == 0)
    {
	GArray *result_array = g_array_new(FALSE, FALSE, sizeof(gchar*));
	
	/* Realise derivations on target machines */	
	if((exit_status = realise(interface, derivation_array, result_array)) == 0)
	{
	    /* Retrieve back the build results */
	    exit_status = retrieve_results(interface, derivation_array, result_array);
	}
	
	/* Cleanup */
	delete_result_array(result_array);
    }
    
    /* Cleanup */
    delete_derivation_array(derivation_array);
    
    /* Return the exit status, which is 0 if everything succeeds */
    return exit_status;
}
