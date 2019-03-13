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

#include "targets-iterator.h"

static int has_next_target(void *data)
{
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)data;
    return has_next_iteration_process(&target_iterator_data->model_iterator_data);
}

static pid_t next_target_process(void *data)
{
    /* Declarations */
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)data;

    /* Retrieve target */
    Target *target = g_ptr_array_index(target_iterator_data->target_array, target_iterator_data->model_iterator_data.index);

    /* Invoke the next distribution item operation process */
    pid_t pid = target_iterator_data->map_target(target_iterator_data->data, target);

    /* Increase the iterator index and update the pid table */
    next_iteration_process(&target_iterator_data->model_iterator_data, pid, target);

    /* Return the pid of the invoked process */
    return pid;
}

static void complete_target_process(void *data, pid_t pid, ProcReact_Status status, int result)
{
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)data;

    /* Retrieve the completed item */
    Target *target = complete_iteration_process(&target_iterator_data->model_iterator_data, pid, status, result);

    /* Invoke callback that handles completion of the target */
    target_iterator_data->complete_target_mapping(target_iterator_data->data, target, status, result);
}

ProcReact_PidIterator create_target_iterator(const GPtrArray *target_array, map_target_function map_target, complete_target_mapping_function complete_target_mapping, void *data)
{
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)g_malloc(sizeof(TargetIteratorData));

    init_model_iterator_data(&target_iterator_data->model_iterator_data, target_array->len);
    target_iterator_data->target_array = target_array;
    target_iterator_data->map_target = map_target;
    target_iterator_data->complete_target_mapping = complete_target_mapping;
    target_iterator_data->data = data;

    return procreact_initialize_pid_iterator(has_next_target, next_target_process, procreact_retrieve_boolean, complete_target_process, target_iterator_data);
}

void destroy_target_iterator(ProcReact_PidIterator *iterator)
{
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)iterator->data;
    destroy_model_iterator_data(&target_iterator_data->model_iterator_data);
    g_free(target_iterator_data);
}

int target_iterator_has_succeeded(const ProcReact_PidIterator *iterator)
{
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)iterator->data;
    return target_iterator_data->model_iterator_data.success;
}
