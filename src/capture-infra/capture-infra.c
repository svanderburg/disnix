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
#include <stdlib.h>
#include <infrastructure.h>
#include <client-interface.h>

typedef struct
{
    gchar *target_name;
    char **config;
}
CapturedConfig;

static ProcReact_Future capture_infra_on_target(void *data, Target *target, gchar *client_interface, gchar *target_key)
{
    return exec_capture_config(client_interface, target_key);
}

static void complete_capture_infra_on_target(void *data, Target *target, gchar *target_key, ProcReact_Future *future, ProcReact_Status status)
{
    GPtrArray *configs_array = (GPtrArray*)data;
    
    if(status != PROCREACT_STATUS_OK || future->result == NULL)
        g_printerr("[target: %s]: Cannot capture the infrastructure!\n", target_key);
    else
    {
        CapturedConfig *config = (CapturedConfig*)g_malloc(sizeof(CapturedConfig));
        config->target_name = target->name;
        config->config = future->result;
        
        g_ptr_array_add(configs_array, config);
    }
}

static gint compare_captured_config(const void *l, const void *r)
{
    const CapturedConfig *left = *((CapturedConfig **)l);
    const CapturedConfig *right = *((CapturedConfig **)r);
    
    return g_strcmp0(left->target_name, right->target_name);
}

static void print_configs_array(GPtrArray *configs_array)
{
    unsigned int i;
    
    /* Sort the captured configs so that the overall result is always displayed in a deterministic order */
    g_ptr_array_sort(configs_array, compare_captured_config);
    
    /* Print the captured configs per machine */
    g_print("{\n");
    
    for(i = 0; i < configs_array->len; i++)
    {
        CapturedConfig *config = g_ptr_array_index(configs_array, i);
        char *line;
        unsigned int count = 0;
        
        g_print("  \"%s\" = ", config->target_name);
        
        /* Iterate over each line of the captured config */
        while((line = config->config[count]) != NULL)
        {
            g_print("%s", line);
            count++;
            
            if(config->config[count] != NULL)
                g_print("\n  ");
        }
        
        g_print(";\n");
    }
    
    g_print("}\n");
}

static void delete_configs_array(GPtrArray *configs_array)
{
    unsigned int i;
    
    for(i = 0; i < configs_array->len; i++)
    {
        CapturedConfig *config = g_ptr_array_index(configs_array, i);
        procreact_free_string_array(config->config);
        g_free(config);
    }
    
    g_ptr_array_free(configs_array, TRUE);
}

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
        int success;
        GPtrArray *configs_array = g_ptr_array_new();
        
        /* Iterate over targets and capture their infrastructure configurations */
        ProcReact_FutureIterator iterator = create_target_future_iterator(target_array, target_property, interface, capture_infra_on_target, complete_capture_infra_on_target, configs_array);
        procreact_fork_in_parallel_buffer_and_wait(&iterator);
        success = target_iterator_has_succeeded(iterator.data);
        
        /* Print the captured configurations */
        print_configs_array(configs_array);
        
        /* Cleanup */
        destroy_target_future_iterator(&iterator);
        delete_target_array(target_array);
        delete_configs_array(configs_array);
        
        /* Return exit status */
        return (!success);
    }
}
