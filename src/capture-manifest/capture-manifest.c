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
#include "capture-manifest.h"
#include <stdio.h>
#include <infrastructure.h>
#include <client-interface.h>
#include <profilemanifest.h>

typedef struct
{
    gchar *target_key;
    gchar *derivation;
}
ProfileDerivation;

typedef struct
{
    gchar *profile_path;
    GPtrArray *profiles_array;
}
QueryRequisitesData;

static ProcReact_Future query_requisites_on_target(void *data, Target *target, gchar *client_interface, gchar *target_key)
{
    QueryRequisitesData *query_requisites_data = (QueryRequisitesData*)data;
    return exec_query_requisites(client_interface, target_key, query_requisites_data->profile_path);
}

static void complete_query_requisites_on_target(void *data, Target *target, gchar *target_key, ProcReact_Future *future, ProcReact_Status status)
{
    QueryRequisitesData *query_requisites_data = (QueryRequisitesData*)data;
    
    if(status != PROCREACT_STATUS_OK || future->result == NULL)
        g_printerr("[target: %s]: Cannot query the requisites of the profile!\n", target_key);
    else
    {
        ProfileDerivation *derivation = (ProfileDerivation*)g_malloc(sizeof(ProfileDerivation));
        char **result = (char**)future->result;
        gint result_length = g_strv_length(result);
        
        derivation->target_key = target_key;
        
        if(result_length > 0)
        {
            derivation->derivation = result[result_length - 1];
            result[result_length - 1] = NULL;
        }
        else
            derivation->derivation = NULL;
        
        g_ptr_array_add(query_requisites_data->profiles_array, derivation);
        
        procreact_free_string_array(future->result);
    }
}

static void delete_requisites_data(QueryRequisitesData *query_requisites_data)
{
    g_free(query_requisites_data->profile_path);
}

static int resolve_profiles(GPtrArray *target_array, gchar *interface, const gchar *target_property, gchar *profile, GPtrArray *profiles_array)
{
    int success;
    
    gchar *profile_path = g_strconcat(LOCALSTATEDIR, "/nix/profiles/disnix/", profile, NULL);
    QueryRequisitesData data = { profile_path, profiles_array };
    
    ProcReact_FutureIterator iterator = create_target_future_iterator(target_array, target_property, interface, query_requisites_on_target, complete_query_requisites_on_target, &data);
    
    g_printerr("[coordinator]: Resolving target profile paths...\n");
    
    procreact_fork_in_parallel_buffer_and_wait(&iterator);
    success = target_iterator_has_succeeded(iterator.data);
    
    destroy_target_future_iterator(&iterator);
    delete_requisites_data(&data);
    return success;
}

typedef struct
{
    unsigned int index;
    unsigned int length;
    int success;
    gchar *interface;
    GPtrArray *profiles_array;
}
RetrieveProfilesIteratorData;

static int has_next_profile_derivation(void *data)
{
    RetrieveProfilesIteratorData *iterator_data = (RetrieveProfilesIteratorData*)data;
    return iterator_data->index < iterator_data->length;
}

static pid_t next_profile_derivation(void *data)
{
    RetrieveProfilesIteratorData *iterator_data = (RetrieveProfilesIteratorData*)data;
    ProfileDerivation *derivation = g_ptr_array_index(iterator_data->profiles_array, iterator_data->index);
    gchar *paths[] = { derivation->derivation, NULL };
    
    pid_t pid = exec_copy_closure_from(iterator_data->interface, derivation->target_key, paths);
    iterator_data->index++;
    return pid;
}

static void complete_profile_derivation(void *data, pid_t pid, ProcReact_Status status, int result)
{
    RetrieveProfilesIteratorData *iterator_data = (RetrieveProfilesIteratorData*)data;

    if(status != PROCREACT_STATUS_OK || !result)
    {
        g_printerr("Cannot retrieve profile derivation!\n");
        iterator_data->success = FALSE;
    }
}

static int retrieve_profiles(gchar *interface, GPtrArray *profiles_array, const unsigned int max_concurrent_transfers)
{
    RetrieveProfilesIteratorData data = { 0, profiles_array->len, TRUE, interface, profiles_array };
    ProcReact_PidIterator iterator = procreact_initialize_pid_iterator(has_next_profile_derivation, next_profile_derivation, procreact_retrieve_boolean, complete_profile_derivation, &data);
    
    g_printerr("[coordinator]: Retrieving intra-dependency closures of the profiles...\n");
    
    procreact_fork_and_wait_in_parallel_limit(&iterator, max_concurrent_transfers);
    
    return data.success;
}

static void print_profiles_array(GPtrArray *profiles_array)
{
    unsigned int i;
    
    g_print("[\n");
    
    for(i = 0; i < profiles_array->len; i++)
    {
        ProfileDerivation *derivation = g_ptr_array_index(profiles_array, i);
        g_print("  { profile = builtins.storePath %s; target = \"%s\"; }\n", derivation->derivation, derivation->target_key);
    }
    
    g_print("]");
}

static void print_services_per_target(GPtrArray *profiles_array)
{
    unsigned int i;
    
    g_print("[\n");
    
    for(i = 0; i < profiles_array->len; i++)
    {
        ProfileDerivation *derivation = g_ptr_array_index(profiles_array, i);
        
        if(derivation->derivation != NULL)
        {
            gchar *manifest_file = g_strconcat(derivation->derivation, "/manifest", NULL);
            GPtrArray *profile_manifest_array = create_profile_manifest_array_from_file(manifest_file);
            
            g_print("    { target = \"%s\";\n", derivation->target_key);
            g_print("      services = ");
            print_nix_expression_from_profile_manifest_array(profile_manifest_array);
            g_print(";\n");
            g_print("    }\n");
            
            delete_profile_manifest_array(profile_manifest_array);
            g_free(manifest_file);
        }
    }
    
    g_print("  ]");
}

static void print_captured_config(GPtrArray *profiles_array, gchar *interface, const gchar *target_property, gchar *infrastructure_expr)
{
    g_print("{\n");
    g_print("  profiles = ");
    print_profiles_array(profiles_array);
    g_print(";\n");
    g_print("  servicesPerTarget = ");
    print_services_per_target(profiles_array);
    g_print(";\n");
    g_print("}\n");
}

static void delete_profiles_array(GPtrArray *profiles_array)
{
    unsigned int i;
    
    for(i = 0; i < profiles_array->len; i++)
    {
        ProfileDerivation *derivation = g_ptr_array_index(profiles_array, i);
        free(derivation->derivation);
        g_free(derivation);
    }
    
    g_ptr_array_free(profiles_array, TRUE);
}

int capture_manifest(gchar *interface, const gchar *target_property, gchar *infrastructure_expr, gchar *profile, const unsigned int max_concurrent_transfers)
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
        GPtrArray *profiles_array = g_ptr_array_new();
        int exit_status;
        
        if(resolve_profiles(target_array, interface, target_property, profile, profiles_array)
          && retrieve_profiles(interface, profiles_array, max_concurrent_transfers))
        {
            print_captured_config(profiles_array, interface, target_property, infrastructure_expr);
            exit_status = 0;
        }
        else
            exit_status = 1;
        
        /* Cleanup */
        delete_profiles_array(profiles_array);
        delete_target_array(target_array);
        
        /* Return exit status */
        return exit_status;
    }
}
