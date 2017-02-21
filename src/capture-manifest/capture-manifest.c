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
#include <profilemanifesttarget.h>

/* Resolve profiles infrastructure */

typedef struct
{
    gchar *profile_path;
    GPtrArray *profile_manifest_target_array;
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
        ProfileManifestTarget *profile_manifest_target = (ProfileManifestTarget*)g_malloc(sizeof(ProfileManifestTarget));
        char **result = (char**)future->result;
        gint result_length = g_strv_length(result);
        
        profile_manifest_target->target_key = target_key;
        
        if(result_length > 0)
        {
            profile_manifest_target->derivation = result[result_length - 1];
            result[result_length - 1] = NULL;
        }
        else
            profile_manifest_target->derivation = NULL;
        
        parse_manifest(profile_manifest_target);
        
        g_ptr_array_add(query_requisites_data->profile_manifest_target_array, profile_manifest_target);
        
        procreact_free_string_array(future->result);
    }
}

static int resolve_profiles(GPtrArray *target_array, gchar *interface, const gchar *target_property, gchar *profile, GPtrArray *profile_manifest_target_array)
{
    int success;
    
    gchar *profile_path = g_strconcat(LOCALSTATEDIR "/nix/profiles/disnix/", profile, NULL);
    QueryRequisitesData data = { profile_path, profile_manifest_target_array };
    
    ProcReact_FutureIterator iterator = create_target_future_iterator(target_array, target_property, interface, query_requisites_on_target, complete_query_requisites_on_target, &data);
    
    g_printerr("[coordinator]: Resolving target profile paths...\n");
    
    procreact_fork_in_parallel_buffer_and_wait(&iterator);
    success = target_iterator_has_succeeded(iterator.data);
    
    /* Sort the profiles array to make the outcome deterministic */
    g_ptr_array_sort(data.profile_manifest_target_array, compare_profile_manifest_target);
    
    /* Cleanup */
    destroy_target_future_iterator(&iterator);
    g_free(data.profile_path);
    
    /* Return success status */
    return success;
}

/* Retrieve profiles infrastructure */

pid_t retrieve_profile_manifest_target(void *data, ProfileManifestTarget *profile_manifest_target)
{
    gchar *interface = (gchar*)data;
    gchar *paths[] = { profile_manifest_target->derivation, NULL };
    
    return exec_copy_closure_from(interface, profile_manifest_target->target_key, paths);
}

void complete_retrieve_profile_manifest_target(void *data, ProfileManifestTarget *profile_manifest_target, ProcReact_Status status, int result)
{
    if(status != PROCREACT_STATUS_OK || !result)
        g_printerr("[target: %s]: Cannot retrieve intra-dependency closure of profile!\n", profile_manifest_target->target_key);
}

static int retrieve_profiles(gchar *interface, GPtrArray *profile_manifest_target_array, const unsigned int max_concurrent_transfers)
{
    int success;
    ProcReact_PidIterator iterator = create_profile_manifest_target_iterator(profile_manifest_target_array, retrieve_profile_manifest_target, complete_retrieve_profile_manifest_target, interface);
    
    g_printerr("[coordinator]: Retrieving intra-dependency closures of the profiles...\n");
    
    procreact_fork_and_wait_in_parallel_limit(&iterator, max_concurrent_transfers);
    success = profile_manifest_target_iterator_has_succeeded(&iterator);
    destroy_profile_manifest_target_iterator(&iterator);
    
    return success;
}

/* The entire capture manifest operation */

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
        GPtrArray *profile_manifest_target_array = g_ptr_array_new();
        int exit_status;
        
        if(resolve_profiles(target_array, interface, target_property, profile, profile_manifest_target_array)
          && retrieve_profiles(interface, profile_manifest_target_array, max_concurrent_transfers))
        {
            print_nix_expression_for_profile_manifest_target_array(profile_manifest_target_array);
            exit_status = 0;
        }
        else
            exit_status = 1;
        
        /* Cleanup */
        delete_profile_manifest_target_array(profile_manifest_target_array);
        delete_target_array(target_array);
        
        /* Return exit status */
        return exit_status;
    }
}
