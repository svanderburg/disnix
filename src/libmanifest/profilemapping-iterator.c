/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2021  Sander van der Burg
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

#include "profilemapping-iterator.h"

static int has_next_profile_mapping(void *data)
{
    ProfileMappingIteratorData *profile_mapping_iterator_data = (ProfileMappingIteratorData*)data;
    return has_next_iteration_process(&profile_mapping_iterator_data->model_iterator_data);
}

static pid_t next_profile_mapping_process(void *data)
{
    /* Declarations */
    ProfileMappingIteratorData *profile_mapping_iterator_data = (ProfileMappingIteratorData*)data;

    /* Retrieve profile mapping, target pair */
    void *key, *value;
    g_hash_table_iter_next(&profile_mapping_iterator_data->iter, &key, &value);
    gchar *target_name = (gchar*)key;
    xmlChar *profile_path = (xmlChar*)value;
    Target *target = g_hash_table_lookup(profile_mapping_iterator_data->targets_table, target_name);

    /* Invoke the next profile mapping operation process */
    pid_t pid = profile_mapping_iterator_data->map_profile_mapping(profile_mapping_iterator_data->data, target_name, profile_path, target);

    /* Increase the iterator index and update the pid table */
    next_iteration_process(&profile_mapping_iterator_data->model_iterator_data, pid, target_name);

    /* Return the pid of the invoked process */
    return pid;
}

static void complete_profile_mapping_process(void *data, pid_t pid, ProcReact_Status status, int result)
{
    ProfileMappingIteratorData *profile_mapping_iterator_data = (ProfileMappingIteratorData*)data;

    /* Retrieve the completed item */
    gchar *target_name = complete_iteration_process(&profile_mapping_iterator_data->model_iterator_data, pid, status, result);
    xmlChar *profile_path = g_hash_table_lookup(profile_mapping_iterator_data->profile_mapping_table, target_name);
    Target *target = g_hash_table_lookup(profile_mapping_iterator_data->targets_table, target_name);

    /* Invoke callback that handles completion of the profile mapping */
    profile_mapping_iterator_data->complete_map_profile_mapping(profile_mapping_iterator_data->data, target_name, profile_path, target, status, result);
}

ProcReact_PidIterator create_profile_mapping_iterator(GHashTable *profile_mapping_table, GHashTable *targets_table, map_profile_mapping_function map_profile_mapping, complete_map_profile_mapping_function complete_map_profile_mapping, void *data)
{
    ProfileMappingIteratorData *profile_mapping_iterator_data = (ProfileMappingIteratorData*)g_malloc(sizeof(ProfileMappingIteratorData));

    init_model_iterator_data(&profile_mapping_iterator_data->model_iterator_data, g_hash_table_size(profile_mapping_table));
    profile_mapping_iterator_data->profile_mapping_table = profile_mapping_table;
    g_hash_table_iter_init(&profile_mapping_iterator_data->iter, profile_mapping_iterator_data->profile_mapping_table);
    profile_mapping_iterator_data->targets_table = targets_table;
    profile_mapping_iterator_data->map_profile_mapping = map_profile_mapping;
    profile_mapping_iterator_data->complete_map_profile_mapping = complete_map_profile_mapping;
    profile_mapping_iterator_data->data = data;

    return procreact_initialize_pid_iterator(has_next_profile_mapping, next_profile_mapping_process, procreact_retrieve_boolean, complete_profile_mapping_process, profile_mapping_iterator_data);
}

void destroy_profile_mapping_iterator(ProcReact_PidIterator *iterator)
{
    ProfileMappingIteratorData *profile_mapping_iterator_data = (ProfileMappingIteratorData*)iterator->data;
    destroy_model_iterator_data(&profile_mapping_iterator_data->model_iterator_data);
    g_free(profile_mapping_iterator_data);
}

ProcReact_bool profile_mapping_iterator_has_succeeded(const ProcReact_PidIterator *iterator)
{
    ProfileMappingIteratorData *profile_mapping_iterator_data = (ProfileMappingIteratorData*)iterator->data;
    return profile_mapping_iterator_data->model_iterator_data.success;
}
