/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2018  Sander van der Burg
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
#include <profilemanifesttarget.h>
#include <profilemanifest.h>

typedef struct
{
    gchar *profile;
    GPtrArray *profile_manifest_target_array;
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
        ProfileManifestTarget *profile_manifest_target = (ProfileManifestTarget*)g_malloc(sizeof(ProfileManifestTarget));
        profile_manifest_target->target_key = target_key;
        profile_manifest_target->derivation = NULL;
        profile_manifest_target->profile_manifest_array = create_profile_manifest_array_from_string_array(future->result);
        
        g_ptr_array_add(query_installed_services_data->profile_manifest_target_array, profile_manifest_target);
        
        free(future->result);
    }
}

static void print_installed_services(QueryInstalledServicesData *query_installed_services_data, OutputFormat format)
{
    /* Sort the captured configs so that the overall result is always displayed in a deterministic order */
    g_ptr_array_sort(query_installed_services_data->profile_manifest_target_array, compare_profile_manifest_target);
    
    switch(format)
    {
        case FORMAT_SERVICES:
            print_services_in_profile_manifest_target(query_installed_services_data->profile_manifest_target_array);
            break;
        case FORMAT_CONTAINERS:
            print_services_per_container_in_profile_manifest_target(query_installed_services_data->profile_manifest_target_array);
            break;
        case FORMAT_NIX:
            print_nix_expression_from_services_in_profile_manifest_target(query_installed_services_data->profile_manifest_target_array);
            break;
    }
}

int query_installed(gchar *interface, const gchar *target_property, gchar *infrastructure_expr, gchar *profile, OutputFormat format, const int xml)
{
    /* Retrieve an array of all target machines from the infrastructure expression */
    GPtrArray *target_array = create_target_array(infrastructure_expr, xml);

    if(target_array == NULL)
    {
        g_printerr("[coordinator]: Error retrieving targets from infrastructure model!\n");
        return 1;
    }
    else
    {
        int success;

        /* Iterate over targets and capture their installed services */
        GPtrArray *profile_manifest_target_array = g_ptr_array_new();
        QueryInstalledServicesData data = { profile, profile_manifest_target_array };
        ProcReact_FutureIterator iterator = create_target_future_iterator(target_array, target_property, interface, query_installed_services_on_target, complete_query_installed_services_on_target, &data);
        procreact_fork_in_parallel_buffer_and_wait(&iterator);
        success = target_iterator_has_succeeded(iterator.data);
        
        /* Print the captured configurations */
        print_installed_services(&data, format);
        
        /* Cleanup */
        destroy_target_future_iterator(&iterator);
        delete_profile_manifest_target_array(profile_manifest_target_array);
        delete_target_array(target_array);
        
        /* Return exit status */
        return (!success);
    }
}
