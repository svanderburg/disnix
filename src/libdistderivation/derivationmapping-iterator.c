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

#include "derivationmapping-iterator.h"

static int has_next_derivation_mapping(void *data)
{
    DerivationIteratorData *derivation_iterator_data = (DerivationIteratorData*)data;
    return has_next_iteration_process(&derivation_iterator_data->model_iterator_data);
}

static pid_t next_derivation_process(void *data)
{
    /* Declarations */
    DerivationIteratorData *derivation_iterator_data = (DerivationIteratorData*)data;

    /* Retrieve derivation item, interface pair */
    DerivationMapping *mapping = g_ptr_array_index(derivation_iterator_data->derivation_mapping_array, derivation_iterator_data->model_iterator_data.index);
    Interface *interface = g_hash_table_lookup(derivation_iterator_data->interfaces_table, (gchar*)mapping->interface);

    /* Invoke the next derivation mapping operation process */
    pid_t pid = derivation_iterator_data->map_derivation_mapping_function.pid(derivation_iterator_data->data, mapping, interface);

    /* Increase the iterator index and update the pid table */
    next_iteration_process(&derivation_iterator_data->model_iterator_data, pid, mapping);

    /* Return the pid of the invoked process */
    return pid;
}

static void complete_derivation_process(void *data, pid_t pid, ProcReact_Status status, int result)
{
    DerivationIteratorData *derivation_iterator_data = (DerivationIteratorData*)data;

    /* Retrieve the completed mapping */
    DerivationMapping *mapping = complete_iteration_process(&derivation_iterator_data->model_iterator_data, pid, status, result);

    /* Invoke callback that handles the completion of derivation item mapping */
    derivation_iterator_data->complete_derivation_mapping_function.pid(derivation_iterator_data->data, mapping, status, result);
}

static DerivationIteratorData *create_common_iterator(const GPtrArray *derivation_mapping_array, GHashTable *interfaces_table, void *data)
{
    DerivationIteratorData *derivation_iterator_data = (DerivationIteratorData*)g_malloc(sizeof(DerivationIteratorData));

    init_model_iterator_data(&derivation_iterator_data->model_iterator_data, derivation_mapping_array->len);
    derivation_iterator_data->derivation_mapping_array = derivation_mapping_array;
    derivation_iterator_data->interfaces_table = interfaces_table;
    derivation_iterator_data->data = data;

    return derivation_iterator_data;
}

ProcReact_PidIterator create_derivation_pid_iterator(const GPtrArray *derivation_mapping_array, GHashTable *interfaces_table, map_derivation_mapping_pid_function map_derivation_mapping, complete_derivation_mapping_pid_function complete_derivation_mapping, void *data)
{
    DerivationIteratorData *derivation_iterator_data = create_common_iterator(derivation_mapping_array, interfaces_table, data);
    derivation_iterator_data->map_derivation_mapping_function.pid = map_derivation_mapping;
    derivation_iterator_data->complete_derivation_mapping_function.pid = complete_derivation_mapping;

    return procreact_initialize_pid_iterator(has_next_derivation_mapping, next_derivation_process, procreact_retrieve_boolean, complete_derivation_process, derivation_iterator_data);
}

static ProcReact_Future next_derivation_future(void *data)
{
    /* Declarations */
    DerivationIteratorData *derivation_iterator_data = (DerivationIteratorData*)data;

    /* Retrieve derivation item, interface pair */
    DerivationMapping *mapping = g_ptr_array_index(derivation_iterator_data->derivation_mapping_array, derivation_iterator_data->model_iterator_data.index);
    Interface *interface = g_hash_table_lookup(derivation_iterator_data->interfaces_table, (gchar*)mapping->interface);

    /* Invoke the next derivation item operation process */
    ProcReact_Future future = derivation_iterator_data->map_derivation_mapping_function.future(derivation_iterator_data->data, mapping, interface);

    /* Increase the iterator and update the pid table */
    next_iteration_future(&derivation_iterator_data->model_iterator_data, &future, mapping);

    /* Return the pid of the invoked process */
    return future;
}

static void complete_derivation_future(void *data, ProcReact_Future *future, ProcReact_Status status)
{
    DerivationIteratorData *derivation_iterator_data = (DerivationIteratorData*)data;

    /* Retrieve the completed mapping */
    DerivationMapping *mapping = complete_iteration_future(&derivation_iterator_data->model_iterator_data, future, status);

    /* Invoke callback that handles the completion of derivation item mapping */
    derivation_iterator_data->complete_derivation_mapping_function.future(derivation_iterator_data->data, mapping, future, status);
}

ProcReact_FutureIterator create_derivation_future_iterator(const GPtrArray *derivation_mapping_array, GHashTable *interfaces_table, map_derivation_mapping_future_function map_derivation_mapping, complete_derivation_mapping_future_function complete_derivation_mapping, void *data)
{
    DerivationIteratorData *derivation_iterator_data = create_common_iterator(derivation_mapping_array, interfaces_table, data);
    derivation_iterator_data->map_derivation_mapping_function.future = map_derivation_mapping;
    derivation_iterator_data->complete_derivation_mapping_function.future = complete_derivation_mapping;

    return procreact_initialize_future_iterator(has_next_derivation_mapping, next_derivation_future, complete_derivation_future, derivation_iterator_data);
}

static void destroy_derivation_iterator_data(DerivationIteratorData *derivation_iterator_data)
{
    destroy_model_iterator_data(&derivation_iterator_data->model_iterator_data);
    g_free(derivation_iterator_data);
}

void destroy_derivation_pid_iterator(ProcReact_PidIterator *iterator)
{
    DerivationIteratorData *derivation_iterator_data = (DerivationIteratorData*)iterator->data;
    destroy_derivation_iterator_data(derivation_iterator_data);
}

void destroy_derivation_future_iterator(ProcReact_FutureIterator *iterator)
{
    DerivationIteratorData *derivation_iterator_data = (DerivationIteratorData*)iterator->data;
    destroy_derivation_iterator_data(derivation_iterator_data);
    procreact_destroy_future_iterator(iterator);
}

int derivation_iterator_has_succeeded(const DerivationIteratorData *derivation_iterator_data)
{
    return derivation_iterator_data->model_iterator_data.success;
}
