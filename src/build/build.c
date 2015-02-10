/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2013  Sander van der Burg
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

#include <stdio.h>
#include <unistd.h>

#include <derivationmapping.h>
#include <client-interface.h>

#define BUFFER_SIZE 4096

static int distribute_derivations(gchar *interface, const GPtrArray *derivation_array)
{
    int status = 0;
    unsigned int i;
    
    for(i = 0; i < derivation_array->len; i++)
    {   
        /* Retrieve derivation item from array */
        DerivationItem *item = g_ptr_array_index(derivation_array, i);
        
        /* Execute copy closure process */
        g_print("[target: %s]: Receiving intra-dependency closure of store derivation: %s\n", item->target, item->derivation);
        status = wait_to_finish(exec_copy_closure_to(interface, item->target, item->derivation));
        
        /* On error stop the distribution process */
        if(status != 0)
            return status;
    }
    
    return status;
}

static int realise(gchar *interface, GPtrArray *derivation_array, GPtrArray *result_array)
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
        DerivationItem *item = g_ptr_array_index(derivation_array, i);
        
        g_print("[target: %s]: Realising derivation: %s\n", item->target, item->derivation);
        status = exec_realise(interface, item->target, item->derivation, pipefd);
        
        if(status == -1)
        {
            g_printerr("[target: %s]: Error with forking realise process!\n", item->target);
            exit_status = -1;
        }
        else
        {
            g_array_append_val(output_array, pipefd[0]); /* Append read file descriptor to array */
            running_processes++;
        }
    }
    
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
                g_ptr_array_insert(result_array, -1, result);
            }
        }
        
        close(pipefd);
    }

    /* Check statusses of the running processes */
    for(i = 0; i < running_processes; i++)
    {
        status = wait_to_finish(0);

        /* If one of the processes fail, change the exit status */
        if(status != 0)
        {
            g_printerr("Error with executing realise process!\n");
            exit_status = status;
        }
    }
    
    /* Cleanup */
    g_array_free(output_array, TRUE);
    
    /* Return the exit status, which is 0 if everything succeeds */
    return exit_status;
}

static int retrieve_results(gchar *interface, GPtrArray *derivation_array, GPtrArray *result_array)
{
    unsigned int i;
    int status = 0;
    
    /* Iterate over the derivation array and copy the build results back */
    for(i = 0; i < derivation_array->len; i++)
    {
        gchar *result;
        DerivationItem *item;
        
        result = g_ptr_array_index(result_array, i);
        item = g_ptr_array_index(derivation_array, i);
        
        g_print("[target: %s]: Sending build result to coordinator: %s\n", item->target, result);
        
        status = wait_to_finish(exec_copy_closure_from(interface, item->target, result));
        
        /* On error stop build process */
        if(status != 0)
            return status;
    }
    
    return status;
}

static void delete_result_array(GPtrArray *result_array)
{
    if(result_array != NULL)
    {
        unsigned int i;
        
        for(i = 0; i < result_array->len; i++)
        {
            gchar *result = g_ptr_array_index(result_array, i);
            g_free(result);
        }
    
        g_ptr_array_free(result_array, TRUE);
    }
}

static void cleanup(GPtrArray *result_array, GPtrArray *derivation_array)
{
    delete_result_array(result_array);
    delete_derivation_array(derivation_array);
}

int build(gchar *interface, const gchar *distributed_derivation_file)
{
    GPtrArray *derivation_array = create_derivation_array(distributed_derivation_file);
    
    if(derivation_array == NULL)
    {
        g_printerr("[coordinator]: Cannot find any derivation mappings!\n");
        return 1;
    }
    else
    {
        int status;
        GPtrArray *result_array = g_ptr_array_new();
        
        /* Distribute derivations to target machines */
        if((status = distribute_derivations(interface, derivation_array)) != 0)
        {
             cleanup(result_array, derivation_array);
             return status;
        }
        
        /* Realise derivations on target machines */
        if((status = realise(interface, derivation_array, result_array)) != 0)
        {
            cleanup(result_array, derivation_array);
            return status;
        }
        
        /* Retrieve back the build results */
        if((status = retrieve_results(interface, derivation_array, result_array)) != 0)
        {
            cleanup(result_array, derivation_array);
            return status;
        }
        
        /* Cleanup */
        cleanup(result_array, derivation_array);
        
        /* Return the exit status, which is 0 if everything succeeds */
        return 0;
    }
}
