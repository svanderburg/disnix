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

#include "profilemanifesttarget.h"
#include "profilemanifest.h"

gint compare_profile_manifest_target(const void *l, const void *r)
{
    const ProfileManifestTarget *left = *((ProfileManifestTarget **)l);
    const ProfileManifestTarget *right = *((ProfileManifestTarget **)r);
    
    return g_strcmp0(left->target_key, right->target_key);
}

void parse_manifest(ProfileManifestTarget *profile_manifest_target)
{
    gchar *manifest_file = g_strconcat(profile_manifest_target->derivation, "/manifest", NULL);
    profile_manifest_target->profile_manifest_array = create_profile_manifest_array_from_file(manifest_file);
    g_free(manifest_file);
}

void delete_profile_manifest_target_array(GPtrArray *profile_manifest_target_array)
{
    unsigned int i;
    
    for(i = 0; i < profile_manifest_target_array->len; i++)
    {
        ProfileManifestTarget *profile_manifest_target = g_ptr_array_index(profile_manifest_target_array, i);
        g_free(profile_manifest_target->derivation);
        delete_profile_manifest_array(profile_manifest_target->profile_manifest_array);
        g_free(profile_manifest_target);
    }
    
    g_ptr_array_free(profile_manifest_target_array, TRUE);
}

void print_services_in_profile_manifest_target(const GPtrArray *profile_manifest_target_array)
{
    unsigned int i;

    for(i = 0; i < profile_manifest_target_array->len; i++)
    {
        ProfileManifestTarget *profile_manifest_target = g_ptr_array_index(profile_manifest_target_array, i);

        g_print("\nServices on: %s\n\n", profile_manifest_target->target_key);
        print_services_in_profile_manifest_array(profile_manifest_target->profile_manifest_array);
    }
}

void print_services_per_container_in_profile_manifest_target(const GPtrArray *profile_manifest_target_array)
{
    unsigned int i;

    for(i = 0; i < profile_manifest_target_array->len; i++)
    {
        ProfileManifestTarget *profile_manifest_target = g_ptr_array_index(profile_manifest_target_array, i);

        g_print("\nServices on: %s\n\n", profile_manifest_target->target_key);
        print_services_per_container_in_profile_manifest_array(profile_manifest_target->profile_manifest_array);
    }
}

void print_nix_expression_from_services_in_profile_manifest_target(const GPtrArray *profile_manifest_target_array)
{
    unsigned int i;

    g_print("[\n");

    for(i = 0; i < profile_manifest_target_array->len; i++)
    {
        ProfileManifestTarget *profile_manifest_target = g_ptr_array_index(profile_manifest_target_array, i);

        g_print("  { target = \"%s\";\n", profile_manifest_target->target_key);
        g_print("    services = ");
        print_nix_expression_from_profile_manifest_array(profile_manifest_target->profile_manifest_array);
        g_print(";\n");
        g_print("  }\n");
    }

    g_print("]");
}

void print_nix_expression_from_derivations_in_profile_manifest_array(const GPtrArray *profile_manifest_target_array)
{
    unsigned int i;
    
    g_print("[\n");
    
    for(i = 0; i < profile_manifest_target_array->len; i++)
    {
        ProfileManifestTarget *profile_manifest_target = g_ptr_array_index(profile_manifest_target_array, i);
        g_print("  { profile = builtins.storePath %s; target = \"%s\"; }\n", profile_manifest_target->derivation, profile_manifest_target->target_key);
    }
    
    g_print("]");
}

void print_nix_expression_for_profile_manifest_target_array(const GPtrArray *profile_manifest_target_array)
{
    g_print("{\n");
    g_print("  distribution = ");
    print_nix_expression_from_derivations_in_profile_manifest_array(profile_manifest_target_array);
    g_print(";\n");
    g_print("  servicesPerTarget = ");
    print_nix_expression_from_services_in_profile_manifest_target(profile_manifest_target_array);
    g_print(";\n");
    g_print("}\n");
}

static int has_next_profile_manifest_target_process(void *data)
{
    ProfileManifestTargetIteratorData *iterator_data = (ProfileManifestTargetIteratorData*)data;
    return has_next_iteration_process(&iterator_data->model_iterator_data);
}

static pid_t next_profile_manifest_target_process(void *data)
{
    /* Declarations */
    ProfileManifestTargetIteratorData *iterator_data = (ProfileManifestTargetIteratorData*)data;
    
    /* Retrieve profile manifest target */
    ProfileManifestTarget *profile_manifest_target = g_ptr_array_index(iterator_data->profile_manifest_target_array, iterator_data->model_iterator_data.index);
    
    /* Invoke the next profile manifest target item operation process */
    pid_t pid = iterator_data->map_profilemanifesttarget_item(iterator_data->data, profile_manifest_target);
    
    /* Increase the iterator index and update the pid table */
    next_iteration_process(&iterator_data->model_iterator_data, pid, profile_manifest_target);
    
    /* Return the pid of the invoked process */
    return pid;
}

static void complete_profile_manifest_target_process(void *data, pid_t pid, ProcReact_Status status, int result)
{
    ProfileManifestTargetIteratorData *iterator_data = (ProfileManifestTargetIteratorData*)data;
    
    /* Retrieve the completed item */
    ProfileManifestTarget *item = complete_iteration_process(&iterator_data->model_iterator_data, pid, status, result);
    
    /* Invoke callback that handles completion of distribution item */
    iterator_data->complete_profilemanifesttarget_item_mapping(iterator_data->data, item, status, result);
}

ProcReact_PidIterator create_profile_manifest_target_iterator(GPtrArray *profile_manifest_target_array, map_profilemanifesttarget_item_function map_profilemanifesttarget_item, complete_profilemanifesttarget_item_mapping_function complete_profilemanifesttarget_item_mapping, void *data)
{
    ProfileManifestTargetIteratorData *profile_manifest_target_iterator_data = (ProfileManifestTargetIteratorData*)g_malloc(sizeof(ProfileManifestTargetIteratorData));
    
    init_model_iterator_data(&profile_manifest_target_iterator_data->model_iterator_data, profile_manifest_target_array->len);
    profile_manifest_target_iterator_data->profile_manifest_target_array = profile_manifest_target_array;
    profile_manifest_target_iterator_data->map_profilemanifesttarget_item = map_profilemanifesttarget_item;
    profile_manifest_target_iterator_data->complete_profilemanifesttarget_item_mapping = complete_profilemanifesttarget_item_mapping;
    profile_manifest_target_iterator_data->data = data;
    
    return procreact_initialize_pid_iterator(has_next_profile_manifest_target_process, next_profile_manifest_target_process, procreact_retrieve_boolean, complete_profile_manifest_target_process, profile_manifest_target_iterator_data);
}

void destroy_profile_manifest_target_iterator(ProcReact_PidIterator *iterator)
{
    ProfileManifestTargetIteratorData *iterator_data = (ProfileManifestTargetIteratorData*)iterator->data;
    destroy_model_iterator_data(&iterator_data->model_iterator_data);
    g_free(iterator_data);
}

int profile_manifest_target_iterator_has_succeeded(const ProcReact_PidIterator *iterator)
{
    ProfileManifestTargetIteratorData *iterator_data = (ProfileManifestTargetIteratorData*)iterator->data;
    return iterator_data->model_iterator_data.success;
}
