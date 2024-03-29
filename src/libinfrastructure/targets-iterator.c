/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2022  Sander van der Burg
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

#include "targets-iterator.h"

static int has_next_target(void *data)
{
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)data;
    return has_next_iteration_process(&target_iterator_data->model_iterator_data);
}

static pid_t next_target_process(void *data)
{
    /* Declarations */
    pid_t pid;
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)data;

    /* Retrieve distributionitem, target pair */
    void *key, *value;
    g_hash_table_iter_next(&target_iterator_data->iter, &key, &value);

    /* Invoke the next distribution item operation process */
    pid = target_iterator_data->map_target_function.pid(target_iterator_data->data, (gchar*)key, (Target*)value);

    /* Increase the iterator index and update the pid table */
    next_iteration_process(&target_iterator_data->model_iterator_data, pid, (gchar*)key);

    /* Return the pid of the invoked process */
    return pid;
}

static void complete_target_process(void *data, pid_t pid, ProcReact_Status status, int result)
{
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)data;

    /* Retrieve the completed item */
    gchar *target_name = complete_iteration_process(&target_iterator_data->model_iterator_data, pid, status, result);
    Target *target = g_hash_table_lookup(target_iterator_data->targets_table, target_name);

    /* Invoke callback that handles completion of the target */
    target_iterator_data->complete_target_mapping_function.pid(target_iterator_data->data, target_name, target, status, result);
}

static ProcReact_Future next_target_future(void *data)
{
    /* Declarations */
    ProcReact_Future future;
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)data;

    /* Retrieve distributionitem, target pair */
    void *key, *value;
    g_hash_table_iter_next(&target_iterator_data->iter, &key, &value);
    Target *target = (Target*)value;

    /* Invoke the next distribution item operation process */
    future = target_iterator_data->map_target_function.future(target_iterator_data->data, (gchar*)key, target);

    /* Increase the iterator index and update the pid table */
    next_iteration_future(&target_iterator_data->model_iterator_data, &future, (gchar*)key);

    /* Return the future of the invoked process */
    return future;
}

static void complete_target_future(void *data, ProcReact_Future *future, ProcReact_Status status)
{
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)data;

    /* Retrieve corresponding target and properties of the pid */
    gchar *target_name = complete_iteration_future(&target_iterator_data->model_iterator_data, future, status);
    Target *target = g_hash_table_lookup(target_iterator_data->targets_table, target_name);

    /* Invoke callback that handles completion of the target */
    target_iterator_data->complete_target_mapping_function.future(target_iterator_data->data, target_name, target, future, status);
}

static TargetIteratorData *create_common_iterator(GHashTable *targets_table, void *data)
{
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)g_malloc(sizeof(TargetIteratorData));

    init_model_iterator_data(&target_iterator_data->model_iterator_data, g_hash_table_size(targets_table));
    target_iterator_data->targets_table = targets_table;
    g_hash_table_iter_init(&target_iterator_data->iter, target_iterator_data->targets_table);
    target_iterator_data->data = data;

    return target_iterator_data;
}

ProcReact_PidIterator create_target_pid_iterator(GHashTable *targets_table, map_target_pid_function map_target, complete_target_mapping_pid_function complete_target_mapping, void *data)
{
    TargetIteratorData *target_iterator_data = create_common_iterator(targets_table, data);

    target_iterator_data->map_target_function.pid = map_target;
    target_iterator_data->complete_target_mapping_function.pid = complete_target_mapping;

    return procreact_initialize_pid_iterator(has_next_target, next_target_process, procreact_retrieve_boolean, complete_target_process, target_iterator_data);
}

ProcReact_FutureIterator create_target_future_iterator(GHashTable *targets_table, map_target_future_function map_target, complete_target_mapping_future_function complete_target_mapping, void *data)
{
    TargetIteratorData *target_iterator_data = create_common_iterator(targets_table, data);

    target_iterator_data->map_target_function.future = map_target;
    target_iterator_data->complete_target_mapping_function.future = complete_target_mapping;

    return procreact_initialize_future_iterator(has_next_target, next_target_future, complete_target_future, target_iterator_data);
}

static void destroy_target_iterator_data(TargetIteratorData *target_iterator_data)
{
    destroy_model_iterator_data(&target_iterator_data->model_iterator_data);
    g_free(target_iterator_data);
}

void destroy_target_pid_iterator(ProcReact_PidIterator *iterator)
{
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)iterator->data;
    destroy_target_iterator_data(target_iterator_data);
}

void destroy_target_future_iterator(ProcReact_FutureIterator *iterator)
{
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)iterator->data;
    destroy_target_iterator_data(target_iterator_data);
    procreact_destroy_future_iterator(iterator);
}

ProcReact_bool target_iterator_has_succeeded(const TargetIteratorData *target_iterator_data)
{
    return target_iterator_data->model_iterator_data.success;
}
