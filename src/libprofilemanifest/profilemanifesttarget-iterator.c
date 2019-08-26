/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2019  Sander van der Burg
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

#include "profilemanifesttarget-iterator.h"

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
    void *key, *value;
    g_hash_table_iter_next(&iterator_data->iter, &key, &value);
    ProfileManifestTarget *profile_manifest_target = (ProfileManifestTarget*)value;
    Target *target = g_hash_table_lookup(iterator_data->targets_table, (gchar*)key);

    /* Invoke the next profile manifest target operation process */
    pid_t pid = iterator_data->map_profilemanifesttarget(iterator_data->data, (gchar*)key, profile_manifest_target, target);

    /* Increase the iterator index and update the pid table */
    next_iteration_process(&iterator_data->model_iterator_data, pid, (gchar*)key);

    /* Return the pid of the invoked process */
    return pid;
}

static void complete_profile_manifest_target_process(void *data, pid_t pid, ProcReact_Status status, int result)
{
    ProfileManifestTargetIteratorData *iterator_data = (ProfileManifestTargetIteratorData*)data;

    /* Retrieve the completed profile manifest target */
    gchar *target_name = complete_iteration_process(&iterator_data->model_iterator_data, pid, status, result);
    ProfileManifestTarget *profile_manifest_target = g_hash_table_lookup(iterator_data->profile_manifest_target_table, target_name);
    Target *target = g_hash_table_lookup(iterator_data->targets_table, target_name);

    /* Invoke callback that handles completion a profile manifest target */
    iterator_data->complete_profilemanifesttarget(iterator_data->data, target_name, profile_manifest_target, target, status, result);
}

ProcReact_PidIterator create_profile_manifest_target_iterator(GHashTable *profile_manifest_target_table, map_profilemanifesttarget_function map_profilemanifesttarget, complete_profilemanifesttarget_function complete_profilemanifesttarget, GHashTable *targets_table, void *data)
{
    ProfileManifestTargetIteratorData *profile_manifest_target_iterator_data = (ProfileManifestTargetIteratorData*)g_malloc(sizeof(ProfileManifestTargetIteratorData));

    init_model_iterator_data(&profile_manifest_target_iterator_data->model_iterator_data, g_hash_table_size(profile_manifest_target_table));
    profile_manifest_target_iterator_data->profile_manifest_target_table = profile_manifest_target_table;
    g_hash_table_iter_init(&profile_manifest_target_iterator_data->iter, profile_manifest_target_iterator_data->profile_manifest_target_table);
    profile_manifest_target_iterator_data->map_profilemanifesttarget = map_profilemanifesttarget;
    profile_manifest_target_iterator_data->complete_profilemanifesttarget = complete_profilemanifesttarget;
    profile_manifest_target_iterator_data->targets_table = targets_table;
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
