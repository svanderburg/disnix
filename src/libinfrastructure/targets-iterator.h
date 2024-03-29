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

#ifndef __DISNIX_TARGETS_ITERATOR_H
#define __DISNIX_TARGETS_ITERATOR_H
#include <glib.h>
#include <modeliterator.h>
#include <procreact_pid_iterator.h>
#include <procreact_future_iterator.h>
#include "targetstable.h"

/**
 * Pointer to a function that executes a process for each target
 *
 * @param data An arbitrary data structure
 * @param target_name Name that uniquely identifies the machine
 * @param target A target from the targets table
 * @return The PID of the spawned process
 */
typedef pid_t (*map_target_pid_function) (void *data, gchar *target_name, Target *target);

/**
 * Pointer to a function that constructs a future for each target
 *
 * @param data An arbitrary data structure
 * @param target_name Name that uniquely identifies the machine
 * @param target A target from the targets table
 * @return A future instance
 */
typedef ProcReact_Future (*map_target_future_function) (void *data, gchar *target_name, Target *target);

/**
 * Pointer to a function that gets executed when a process completes for a target
 *
 * @param data An arbitrary data structure
 * @param target_name Name that uniquely identifies the machine
 * @param target A target from the targets table
 * @param status Indicates whether the process terminated abnormally or not
 * @param result TRUE if the operation succeeded, else FALSE
 */
typedef void (*complete_target_mapping_pid_function) (void *data, gchar *target_name, Target *target, ProcReact_Status status, ProcReact_bool result);

/**
 * Pointer to a function that constructs a future when a process completes for a target
 *
 * @param data An arbitrary data structure
 * @param target_name Name that uniquely identifies the machine
 * @param target A target from the targets table
 * @param future The future that has completed
 * @param status Indicates whether the process terminated abnormally or not
 */
typedef void (*complete_target_mapping_future_function) (void *data, gchar *target_name, Target *target, ProcReact_Future *future, ProcReact_Status status);

/**
 * @brief Iterator that can be used to execute a process for each target
 */
typedef struct
{
    /** Common properties for all model iterators */
    ModelIteratorData model_iterator_data;
    /** Iterator for the hash table with targets */
    GHashTableIter iter;
    /** Hash table with targets */
    GHashTable *targets_table;

    /** Function that executes a process for each target */
    union
    {
        /** Pointer to a function that executes a process for each target */
        map_target_pid_function pid;
        /** Pointer to a function that constructs a future for each target */
        map_target_future_function future;
    }
    map_target_function;

    /** Function that gets executed when a process completes for a target */
    union
    {
        /** Pointer to a function that gets executed when a process completes for a target */
        complete_target_mapping_pid_function pid;
        /** Pointer to a function that constructs a future when a process completes for a target */
        complete_target_mapping_future_function future;
    }
    complete_target_mapping_function;

    /** Pointer to arbitrary data passed to the above functions */
    void *data;
}
TargetIteratorData;

/**
 * Creates a new PID iterator that steps over each target and executes the
 * provided functions on start and completion.
 *
 * @param targets_table Hash table with targets
 * @param map_target Function that executes a process for each target
 * @param complete_target_mapping Function that gets executed when a process completes for a target
 * @param data Pointer to arbitrary data passed to the above functions
 * @return A PID iterator that can be used to traverse the targets
 */
ProcReact_PidIterator create_target_pid_iterator(GHashTable *targets_table, map_target_pid_function map_target, complete_target_mapping_pid_function complete_target_mapping, void *data);

/**
 * Creates a new future iterator that steps over each target and executes the
 * provided functions on start and completion.
 *
 * @param targets_table Hash table with targets
 * @param map_target Function that executes a process for each target
 * @param complete_target_mapping Function that gets executed when a process completes for a target
 * @param data Pointer to arbitrary data passed to the above functions
 * @return A future iterator that can be used to traverse the targets
 */
ProcReact_FutureIterator create_target_future_iterator(GHashTable *targets_table, map_target_future_function map_target, complete_target_mapping_future_function complete_target_mapping, void *data);

/**
 * Destroys all resources allocated with a target PID iterator
 *
 * @param iterator A PID iterator instance
 */
void destroy_target_pid_iterator(ProcReact_PidIterator *iterator);

/**
 * Destroys all resources allocated with a target future iterator
 *
 * @param iterator A future iterator instance
 */
void destroy_target_future_iterator(ProcReact_FutureIterator *iterator);

/**
 * Checks whether all iteration steps have succeeded.
 *
 * @param target_iterator_data Struct with properties that facilitate iteration over targets
 * @return TRUE if it indicates success, else FALSE
 */
ProcReact_bool target_iterator_has_succeeded(const TargetIteratorData *target_iterator_data);

#endif
