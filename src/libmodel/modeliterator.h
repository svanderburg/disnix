/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2017  Sander van der Burg
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

#ifndef __DISNIX_MODELITERATOR_H
#define __DISNIX_MODELITERATOR_H

#include <glib.h>
#include <procreact_pid.h>
#include <procreact_future.h>

/**
 * @brief Captures common properties of all model iterators
 */
typedef struct
{
    /** Indicates which element in the array to process */
    unsigned int index;
    /** Contains the length of the array */
    unsigned int length;
    /** Indicates the success status of the iteration */
    int success;
    /** Hash table keeping track with PID belongs to which iteration item */
    GHashTable *pid_table;
}
ModelIteratorData;

/**
 * Intializes a model iterator
 *
 * @param model_iterator_data Model iterator struct instance
 * @param length Length of the collection to traverse
 */
void init_model_iterator_data(ModelIteratorData *model_iterator_data, unsigned int length);

/**
 * Destroys all resources associated with a model iterator
 *
 * @param model_iterator_data Model iterator struct instance
 */
void destroy_model_iterator_data(ModelIteratorData *model_iterator_data);

/**
 * Determines whether there is a next element in the collection
 *
 * @return TRUE if there is a next element, else FALSE
 */
int has_next_iteration_process(ModelIteratorData *model_iterator_data);

/**
 * Increases the iterator index and adds the given item to the pids table so
 * that we can lookup it up when the process completes.
 *
 * @param model_iterator_data Model iterator struct instance
 * @param pid PID of the spawned process
 * @param item Item to add to the PIDs table
 */
void next_iteration_process(ModelIteratorData *model_iterator_data, pid_t pid, gpointer item);

/**
 * Increases the iterator index and adds the given item to the pids table so
 * that we can lookup it up when the process completes.
 *
 * @param model_iterator_data Model iterator struct instance
 * @param future Future struct of a spawned process
 * @param item Item to add to the PIDs table
 */
void next_iteration_future(ModelIteratorData *model_iterator_data, ProcReact_Future *future, gpointer item);

/**
 * Looks up the corresponding item from the PIDs table when a given process
 * completes and evaluates its status.
 *
 * @param model_iterator_data Model iterator struct instance
 * @param pid PID of a completed process
 * @param status Termination status of the finished process
 * @param result Exit status of the process
 * @return Pointer to the item that has been looked up
 */
gpointer complete_iteration_process(ModelIteratorData *model_iterator_data, pid_t pid, ProcReact_Status status, int result);

/**
 * Looks up the corresponding item from the PIDs table when a given process
 * completes and evaluates its status.
 *
 * @param model_iterator_data Model iterator struct instance
 * @param future Future corresponding to a completed process
 * @param status Termination status of the finished process
 * @return Pointer to the item that has been looked up
 */
gpointer complete_iteration_future(ModelIteratorData *model_iterator_data, ProcReact_Future *future, ProcReact_Status status);

#endif
