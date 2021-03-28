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

#include "modeliterator.h"

void init_model_iterator_data(ModelIteratorData *model_iterator_data, unsigned int length)
{
    model_iterator_data->index = 0;
    model_iterator_data->length = length;
    model_iterator_data->success = TRUE;
    model_iterator_data->pid_table = g_hash_table_new_full(g_int_hash, g_int_equal, g_free, NULL);
}

void destroy_model_iterator_data(ModelIteratorData *model_iterator_data)
{
    g_hash_table_destroy(model_iterator_data->pid_table);
}

ProcReact_bool has_next_iteration_process(ModelIteratorData *model_iterator_data)
{
    return model_iterator_data->index < model_iterator_data->length;
}

void next_iteration_process(ModelIteratorData *model_iterator_data, pid_t pid, gpointer item)
{
    /* Increase the iterator index */
    model_iterator_data->index++;

    if(pid > 0)
    {
        /* Add pid to the pid table so that we know what the corresponding item is */
        gint *pid_ptr = g_malloc(sizeof(gint));
        *pid_ptr = pid;
        g_hash_table_insert(model_iterator_data->pid_table, pid_ptr, item);
    }
}

void next_iteration_future(ModelIteratorData *model_iterator_data, ProcReact_Future *future, gpointer item)
{
    return next_iteration_process(model_iterator_data, future->pid, item);
}

gpointer complete_iteration_process(ModelIteratorData *model_iterator_data, pid_t pid, ProcReact_Status status, int result)
{
    /* Retrieve corresponding derivation item of the pid */
    gint *pid_ptr = &pid;
    gpointer item = g_hash_table_lookup(model_iterator_data->pid_table, pid_ptr);

    /* If anything failed, set the overall success status to FALSE */
    if(status != PROCREACT_STATUS_OK || !result)
        model_iterator_data->success = FALSE;

    return item;
}

gpointer complete_iteration_future(ModelIteratorData *model_iterator_data, ProcReact_Future *future, ProcReact_Status status)
{
    /* Retrieve corresponding derivation item of the pid */
    gint *pid_ptr = &future->pid;
    gpointer item = g_hash_table_lookup(model_iterator_data->pid_table, pid_ptr);

    /* If anything failed, set the overall success status to FALSE */
    if(status != PROCREACT_STATUS_OK || future->result == NULL)
        model_iterator_data->success = FALSE;

    return item;
}
