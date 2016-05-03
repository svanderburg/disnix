/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2016  Sander van der Burg
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

#include "capture-infra.h"
#include <stdio.h>
#include <unistd.h>
#include <infrastructure.h>
#include <client-interface.h> 
#define BUFFER_SIZE 4096

int capture_infra(gchar *interface, const gchar *target_property, gchar *infrastructure_expr)
{
    /* Retrieve an array of all target machines from the infrastructure expression */
    GPtrArray *target_array = create_target_array(infrastructure_expr);
    
    if(target_array == NULL)
    {
        g_printerr("[coordinator]: Error retrieving targets from infrastructure model!\n");
        return 1;
    }
    else
    {
        int exit_status = 0;
        unsigned int i;
        
        g_print("{\n");
        
        /* For each target execute the query operation and display the results */
        for(i = 0; i < target_array->len; i++)
        {
            Target *target = g_ptr_array_index(target_array, i);
            gchar *client_interface = target->clientInterface;
            gchar *target_key = find_target_key(target, target_property);
            int status;
            int pipefd[2];
            
            g_print("  \"%s\" = ", target->name);
            
            /* If no client interface is provided by the infrastructure model, use global one */
            if(client_interface == NULL)
                client_interface = interface;
            
            status = exec_capture_config(client_interface, target_key, pipefd);
            
            if(status == -1)
            {
                g_printerr("[target: %s]: Failed executing the capture config!\n", target_key);
                exit_status = status;
            }
            else
            {
                char line[BUFFER_SIZE];
                ssize_t line_size;
        
                while((line_size = read(pipefd[0], line, BUFFER_SIZE - 1)) > 0)
                {
                    gchar **tokens;
                    unsigned int i, length;
                    
                    line[line_size] = '\0';
                    tokens = g_strsplit(line, "\n", 0);
                    
                    length = g_strv_length(tokens);
                    
                    for(i = 0; i < length; i++)
                    {
                        g_print("%s", tokens[i]);
                        
                        if(i < length - 1)
                            g_print("\n  ");
                    }
                    
                    g_strfreev(tokens);
                }
        
                close(pipefd[0]);
            }
            
            status = wait_to_finish(0);
        
            if(status != 0)
            {
                g_printerr("[target: %s]: Failed finishing the capture config operation!\n", target_key);
                exit_status = status;
            }
            
            g_print(";\n");
        }
        
        g_print("}\n");
        
        /* Delete the target array from memory */
        delete_target_array(target_array);
        
        /* Return the exit status, which is 0 if everything succeeds */
        return exit_status;
    }
}
