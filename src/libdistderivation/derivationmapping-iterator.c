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

#include "derivationmapping-iterator.h"

static int has_next_derivation_mapping(void *data)
{
    DerivationMappingIteratorData *derivation_mapping_iterator_data = (DerivationMappingIteratorData*)data;
    return has_next_iteration_process(&derivation_mapping_iterator_data->model_iterator_data);
}

static pid_t next_derivation_mapping_process(void *data)
{
    /* Declarations */
    DerivationMappingIteratorData *derivation_mapping_iterator_data = (DerivationMappingIteratorData*)data;

    /* Retrieve derivation mapping, interface pair */
    DerivationMapping *mapping = g_ptr_array_index(derivation_mapping_iterator_data->derivation_mapping_array, derivation_mapping_iterator_data->model_iterator_data.index);
    Interface *interface = g_hash_table_lookup(derivation_mapping_iterator_data->interfaces_table, (gchar*)mapping->interface);

    /* Invoke the next derivation mapping operation process */
    pid_t pid = derivation_mapping_iterator_data->map_derivation_mapping_function.pid(derivation_mapping_iterator_data->data, mapping, interface);

    /* Increase the iterator index and update the pid table */
    next_iteration_process(&derivation_mapping_iterator_data->model_iterator_data, pid, mapping);

    /* Return the pid of the invoked process */
    return pid;
}

static void complete_derivation_mapping_process(void *data, pid_t pid, ProcReact_Status status, int result)
{
    DerivationMappingIteratorData *derivation_mapping_iterator_data = (DerivationMappingIteratorData*)data;

    /* Retrieve the completed mapping */
    DerivationMapping *mapping = complete_iteration_process(&derivation_mapping_iterator_data->model_iterator_data, pid, status, result);

    /* Invoke callback that handles the completion of derivation mapping */
    derivation_mapping_iterator_data->complete_derivation_mapping_function.pid(derivation_mapping_iterator_data->data, mapping, status, result);
}

static DerivationMappingIteratorData *create_common_iterator(const GPtrArray *derivation_mapping_array, GHashTable *interfaces_table, void *data)
{
    DerivationMappingIteratorData *derivation_mapping_iterator_data = (DerivationMappingIteratorData*)g_malloc(sizeof(DerivationMappingIteratorData));

    init_model_iterator_data(&derivation_mapping_iterator_data->model_iterator_data, derivation_mapping_array->len);
    derivation_mapping_iterator_data->derivation_mapping_array = derivation_mapping_array;
    derivation_mapping_iterator_data->interfaces_table = interfaces_table;
    derivation_mapping_iterator_data->data = data;

    return derivation_mapping_iterator_data;
}

ProcReact_PidIterator create_derivation_mapping_pid_iterator(const GPtrArray *derivation_mapping_array, GHashTable *interfaces_table, map_derivation_mapping_pid_function map_derivation_mapping, complete_derivation_mapping_pid_function complete_derivation_mapping, void *data)
{
    DerivationMappingIteratorData *derivation_mapping_iterator_data = create_common_iterator(derivation_mapping_array, interfaces_table, data);
    derivation_mapping_iterator_data->map_derivation_mapping_function.pid = map_derivation_mapping;
    derivation_mapping_iterator_data->complete_derivation_mapping_function.pid = complete_derivation_mapping;

    return procreact_initialize_pid_iterator(has_next_derivation_mapping, next_derivation_mapping_process, procreact_retrieve_boolean, complete_derivation_mapping_process, derivation_mapping_iterator_data);
}

static ProcReact_Future next_derivation_mapping_future(void *data)
{
    /* Declarations */
    DerivationMappingIteratorData *derivation_mapping_iterator_data = (DerivationMappingIteratorData*)data;

    /* Retrieve derivation mapping, interface pair */
    DerivationMapping *mapping = g_ptr_array_index(derivation_mapping_iterator_data->derivation_mapping_array, derivation_mapping_iterator_data->model_iterator_data.index);
    Interface *interface = g_hash_table_lookup(derivation_mapping_iterator_data->interfaces_table, (gchar*)mapping->interface);

    /* Invoke the next derivation mapping operation process */
    ProcReact_Future future = derivation_mapping_iterator_data->map_derivation_mapping_function.future(derivation_mapping_iterator_data->data, mapping, interface);

    /* Increase the iterator and update the pid table */
    next_iteration_future(&derivation_mapping_iterator_data->model_iterator_data, &future, mapping);

    /* Return the pid of the invoked process */
    return future;
}

static void complete_derivation_mapping_future(void *data, ProcReact_Future *future, ProcReact_Status status)
{
    DerivationMappingIteratorData *derivation_mapping_iterator_data = (DerivationMappingIteratorData*)data;

    /* Retrieve the completed mapping */
    DerivationMapping *mapping = complete_iteration_future(&derivation_mapping_iterator_data->model_iterator_data, future, status);

    /* Invoke callback that handles the completion of derivation mapping */
    derivation_mapping_iterator_data->complete_derivation_mapping_function.future(derivation_mapping_iterator_data->data, mapping, future, status);
}

ProcReact_FutureIterator create_derivation_mapping_future_iterator(const GPtrArray *derivation_mapping_array, GHashTable *interfaces_table, map_derivation_mapping_future_function map_derivation_mapping, complete_derivation_mapping_future_function complete_derivation_mapping, void *data)
{
    DerivationMappingIteratorData *derivation_mapping_iterator_data = create_common_iterator(derivation_mapping_array, interfaces_table, data);
    derivation_mapping_iterator_data->map_derivation_mapping_function.future = map_derivation_mapping;
    derivation_mapping_iterator_data->complete_derivation_mapping_function.future = complete_derivation_mapping;

    return procreact_initialize_future_iterator(has_next_derivation_mapping, next_derivation_mapping_future, complete_derivation_mapping_future, derivation_mapping_iterator_data);
}

static void destroy_derivation_mapping_iterator_data(DerivationMappingIteratorData *derivation_mapping_iterator_data)
{
    destroy_model_iterator_data(&derivation_mapping_iterator_data->model_iterator_data);
    g_free(derivation_mapping_iterator_data);
}

void destroy_derivation_mapping_pid_iterator(ProcReact_PidIterator *iterator)
{
    DerivationMappingIteratorData *derivation_mapping_iterator_data = (DerivationMappingIteratorData*)iterator->data;
    destroy_derivation_mapping_iterator_data(derivation_mapping_iterator_data);
}

void destroy_derivation_mapping_future_iterator(ProcReact_FutureIterator *iterator)
{
    DerivationMappingIteratorData *derivation_mapping_iterator_data = (DerivationMappingIteratorData*)iterator->data;
    destroy_derivation_mapping_iterator_data(derivation_mapping_iterator_data);
    procreact_destroy_future_iterator(iterator);
}

ProcReact_bool derivation_mapping_iterator_has_succeeded(const DerivationMappingIteratorData *derivation_mapping_iterator_data)
{
    return derivation_mapping_iterator_data->model_iterator_data.success;
}
