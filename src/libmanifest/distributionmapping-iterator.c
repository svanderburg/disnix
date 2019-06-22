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

#include "distributionmapping-iterator.h"

static int has_next_distribution_item(void *data)
{
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)data;
    return has_next_iteration_process(&distribution_iterator_data->model_iterator_data);
}

static pid_t next_distribution_process(void *data)
{
    /* Declarations */
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)data;

    /* Retrieve distributionitem, target pair */
    void *key, *value;
    g_hash_table_iter_next(&distribution_iterator_data->iter, &key, &value);
    gchar *target_name = (gchar*)key;
    xmlChar *profile_name = (xmlChar*)value;
    Target *target = g_hash_table_lookup(distribution_iterator_data->targets_table, target_name);

    /* Invoke the next distribution item operation process */
    pid_t pid = distribution_iterator_data->map_distribution_item(distribution_iterator_data->data, profile_name, target_name, target);

    /* Increase the iterator index and update the pid table */
    next_iteration_process(&distribution_iterator_data->model_iterator_data, pid, target_name);

    /* Return the pid of the invoked process */
    return pid;
}

static void complete_distribution_process(void *data, pid_t pid, ProcReact_Status status, int result)
{
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)data;

    /* Retrieve the completed item */
    gchar *target_name = complete_iteration_process(&distribution_iterator_data->model_iterator_data, pid, status, result);
    xmlChar *profile_name = g_hash_table_lookup(distribution_iterator_data->distribution_table, target_name);

    /* Invoke callback that handles completion of distribution item */
    distribution_iterator_data->complete_distribution_item_mapping(distribution_iterator_data->data, profile_name, target_name, status, result);
}

ProcReact_PidIterator create_distribution_iterator(GHashTable *distribution_table, GHashTable *targets_table, map_distribution_item_function map_distribution_item, complete_distribution_item_mapping_function complete_distribution_item_mapping, void *data)
{
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)g_malloc(sizeof(DistributionIteratorData));

    init_model_iterator_data(&distribution_iterator_data->model_iterator_data, g_hash_table_size(distribution_table));
    distribution_iterator_data->distribution_table = distribution_table;
    g_hash_table_iter_init(&distribution_iterator_data->iter, distribution_iterator_data->distribution_table);
    distribution_iterator_data->targets_table = targets_table;
    distribution_iterator_data->map_distribution_item = map_distribution_item;
    distribution_iterator_data->complete_distribution_item_mapping = complete_distribution_item_mapping;
    distribution_iterator_data->data = data;

    return procreact_initialize_pid_iterator(has_next_distribution_item, next_distribution_process, procreact_retrieve_boolean, complete_distribution_process, distribution_iterator_data);
}

void destroy_distribution_iterator(ProcReact_PidIterator *iterator)
{
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)iterator->data;
    destroy_model_iterator_data(&distribution_iterator_data->model_iterator_data);
    g_free(distribution_iterator_data);
}

int distribution_iterator_has_succeeded(const ProcReact_PidIterator *iterator)
{
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)iterator->data;
    return distribution_iterator_data->model_iterator_data.success;
}
