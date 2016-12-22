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
#include "query-installed.h"
#include <infrastructure.h>
#include <client-interface.h>

typedef struct
{
    gchar *target_key;
    char *services;
}
CapturedConfig;

typedef struct
{
    gchar *profile;
    GPtrArray *configs_array;
}
QueryInstalledServicesData;

static ProcReact_Future query_installed_services_on_target(void *data, Target *target, gchar *client_interface, gchar *target_key)
{
    QueryInstalledServicesData *query_installed_services_data = (QueryInstalledServicesData*)data;
    return exec_query_installed(client_interface, target_key, query_installed_services_data->profile);
}

static void complete_query_installed_services_on_target(void *data, Target *target, gchar *target_key, ProcReact_Future *future, ProcReact_Status status)
{
    QueryInstalledServicesData *query_installed_services_data = (QueryInstalledServicesData*)data;
    
    if(status != PROCREACT_STATUS_OK || future->result == NULL)
        g_printerr("[target: %s]: Cannot query the installed services!\n", target_key);
    else
    {
        CapturedConfig *config = (CapturedConfig*)g_malloc(sizeof(CapturedConfig));
        config->target_key = target_key;
        config->services = future->result;
        
        g_ptr_array_add(query_installed_services_data->configs_array, config);
    }
}

static gint compare_captured_config(const void *l, const void *r)
{
    const CapturedConfig *left = *((CapturedConfig **)l);
    const CapturedConfig *right = *((CapturedConfig **)r);
    
    return g_strcmp0(left->target_key, right->target_key);
}

static void print_installed_services(QueryInstalledServicesData *query_installed_services_data)
{
    unsigned int i;
    
    /* Sort the captured configs so that the overall result is always displayed in a deterministic order */
    g_ptr_array_sort(query_installed_services_data->configs_array, compare_captured_config); 
    
    /* Display the services for each target */
    for(i = 0; i < query_installed_services_data->configs_array->len; i++)
    {
        CapturedConfig *config = g_ptr_array_index(query_installed_services_data->configs_array, i);
        g_print("\nServices on: %s\n\n%s", config->target_key, config->services);
    }
}

static void delete_queried_services_data(QueryInstalledServicesData *query_installed_services_data)
{
    unsigned int i;
    
    for(i = 0; i < query_installed_services_data->configs_array->len; i++)
    {
        CapturedConfig *config = g_ptr_array_index(query_installed_services_data->configs_array, i);
        free(config->services);
        g_free(config);
    }
    
    g_ptr_array_free(query_installed_services_data->configs_array, TRUE);
}

int query_installed(gchar *interface, const gchar *target_property, gchar *infrastructure_expr, gchar *profile)
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

        /* Iterate over targets and capture their installed services */
        GPtrArray *configs_array = g_ptr_array_new();
        QueryInstalledServicesData data = { profile, configs_array };
        ProcReact_FutureIterator iterator = create_target_future_iterator(target_array, target_property, interface, query_installed_services_on_target, complete_query_installed_services_on_target, &data);
        procreact_fork_in_parallel_buffer_and_wait(&iterator);
        success = target_iterator_has_succeeded(iterator.data);
        
        /* Print the captured configurations */
        print_installed_services(&data);
        
        /* Cleanup */
        destroy_target_future_iterator(&iterator);
        delete_queried_services_data(&data);
        delete_target_array(target_array);
        
        /* Return exit status */
        return (!success);
    }
}
