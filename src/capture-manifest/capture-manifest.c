/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2020  Sander van der Burg
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
#include <targets-iterator.h>
#include <remote-package-management.h>
#include <profilemanifest.h>
#include <profilemanifesttargettable.h>
#include <profilemanifesttarget-iterator.h>
#include <copy-closure.h>
#include "aggregated-manifest.h"

/* Resolve profiles infrastructure */

typedef struct
{
    gchar *profile_path;
    GHashTable *profile_manifest_target_table;
}
QueryRequisitesData;

static ProcReact_Future query_requisites_on_target(void *data, gchar *target_name, Target *target)
{
    QueryRequisitesData *query_requisites_data = (QueryRequisitesData*)data;
    gchar *target_key = find_target_key(target);
    gchar *derivations[] = { query_requisites_data->profile_path, NULL };

    return exec_query_requisites((char*)target->client_interface, target_key, derivations, 1);
}

static gchar *take_last_element(char **result)
{
    guint result_length = g_strv_length(result);

    if(result_length > 0)
        return g_strdup(result[result_length - 1]);
    else
        return NULL;
}

static void complete_query_requisites_on_target(void *data, gchar *target_name, Target *target, ProcReact_Future *future, ProcReact_Status status)
{
    QueryRequisitesData *query_requisites_data = (QueryRequisitesData*)data;

    if(status != PROCREACT_STATUS_OK || future->result == NULL)
        g_printerr("[target: %s]: Cannot query the requisites of the profile!\n", target_name);
    else
    {
        char **result = (char**)future->result;
        gchar *profile = take_last_element(result);
        ProfileManifestTarget *profile_manifest_target = parse_profile_manifest_target(profile, target_name);

        g_hash_table_insert(query_requisites_data->profile_manifest_target_table, target_name, profile_manifest_target);

        procreact_free_string_array(future->result);
    }
}

static int resolve_profiles(GHashTable *targets_table, gchar *interface, const gchar *target_property, gchar *profile, GHashTable *profile_manifest_target_table)
{
    gchar *profile_path = g_strconcat(LOCALSTATEDIR "/nix/profiles/disnix/", profile, NULL);
    QueryRequisitesData data = { profile_path, profile_manifest_target_table };

    ProcReact_FutureIterator iterator = create_target_future_iterator(targets_table, query_requisites_on_target, complete_query_requisites_on_target, &data);

    g_printerr("[coordinator]: Resolving target profile paths...\n");

    procreact_fork_in_parallel_buffer_and_wait(&iterator);

    /* Cleanup */
    destroy_target_future_iterator(&iterator);
    g_free(data.profile_path);

    /* Return success status. Ignore failures. They get reported, but they are not critical */
    return TRUE;
}

/* Retrieve profiles infrastructure */

static pid_t retrieve_profile_manifest_target(void *data, gchar *target_name, ProfileManifestTarget *profile_manifest_target, Target *target)
{
    gchar *interface = (gchar*)data;
    gchar *paths[] = { profile_manifest_target->profile, NULL };
    gchar *target_key = find_target_key(target);

    return copy_closure_from(interface, target_key, paths, STDOUT_FILENO, STDERR_FILENO);
}

static void complete_retrieve_profile_manifest_target(void *data, gchar *target_name, ProfileManifestTarget *profile_manifest_target, Target *target, ProcReact_Status status, int result)
{
    if(status != PROCREACT_STATUS_OK || !result)
        g_printerr("[target: %s]: Cannot retrieve intra-dependency closure of profile!\n", target_name);
}

static int retrieve_profiles(gchar *interface, GHashTable *profile_manifest_target_table, GHashTable *targets_table, const unsigned int max_concurrent_transfers)
{
    int success;
    ProcReact_PidIterator iterator = create_profile_manifest_target_iterator(profile_manifest_target_table, retrieve_profile_manifest_target, complete_retrieve_profile_manifest_target, targets_table, interface);

    g_printerr("[coordinator]: Retrieving intra-dependency closures of the profiles...\n");

    procreact_fork_and_wait_in_parallel_limit(&iterator, max_concurrent_transfers);
    success = profile_manifest_target_iterator_has_succeeded(&iterator);
    destroy_profile_manifest_target_iterator(&iterator);

    return success;
}

/* The entire capture manifest operation */

int capture_manifest(gchar *interface, gchar *target_property, gchar *infrastructure_expr, gchar *profile, const unsigned int max_concurrent_transfers, const int xml)
{
    /* Retrieve an array of all target machines from the infrastructure expression */
    GHashTable *targets_table = create_targets_table(infrastructure_expr, xml, target_property, interface);

    if(targets_table == NULL)
    {
        g_printerr("[coordinator]: Error retrieving targets from infrastructure model!\n");
        return 1;
    }
    else
    {
        int exit_status;

        if(check_targets_table(targets_table))
        {
            GHashTable *profile_manifest_target_table = g_hash_table_new(g_str_hash, g_str_equal);

            if(resolve_profiles(targets_table, interface, target_property, profile, profile_manifest_target_table)
              && retrieve_profiles(interface, profile_manifest_target_table, targets_table, max_concurrent_transfers)
              && check_profile_manifest_target_table(profile_manifest_target_table))
            {
                Manifest *manifest = aggregate_manifest(profile_manifest_target_table, targets_table);
                print_manifest_nix(stdout, manifest, 0, NULL);
                delete_aggregated_manifest(manifest);
                exit_status = 0;
            }
            else
                exit_status = 1;

            /* Cleanup */
            delete_profile_manifest_target_table(profile_manifest_target_table);
        }
        else
            exit_status = 1;

        delete_targets_table(targets_table);

        /* Return exit status */
        return exit_status;
    }
}
