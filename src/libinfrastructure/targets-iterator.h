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

#ifndef __DISNIX_TARGETS_ITERATOR_H
#define __DISNIX_TARGETS_ITERATOR_H
#include <glib.h>
#include <modeliterator.h>
#include <procreact_pid_iterator.h>
#include <procreact_future_iterator.h>
#include "targetstable.h"

/** Pointer to a function that executes a process for each target */
typedef pid_t (*map_target_pid_function) (void *data, Target *target, gchar *client_interface, gchar *target_key);
/** Pointer to a function that constructs a future for each target */
typedef ProcReact_Future (*map_target_future_function) (void *data, Target *target, gchar *client_interface, gchar *target_key);

/** Pointer to a function that gets executed when a process completes for a target */
typedef void (*complete_target_mapping_pid_function) (void *data, Target *target, gchar *target_key, ProcReact_Status status, int result);
/** Pointer to a function that constructs a future when a process completes for a target */
typedef void (*complete_target_mapping_future_function) (void *data, Target *target, gchar *target_key, ProcReact_Future *future, ProcReact_Status status);

/**
 * @brief Iterator that can be used to execute a process for each target
 */
typedef struct
{
    /** Common properties for all model iterators */
    ModelIteratorData model_iterator_data;
    /** Hash table with targets */
    GHashTableIter iter;
    GHashTable *targets_table;
    /** Global key that is used if no target property is defined by the target machine */
    const gchar *target_property;
    /** Command-line tool that is invoked to reach the target machine */
    gchar *interface;

    /** Function that executes a process for each target */
    union
    {
        /**
         * Pointer to a function that executes a process for each target
         *
         * @param data An arbitrary data structure
         * @param target A target from the target array
         * @param client_interface Executable to invoke to reach the target machine
         * @param target_key Key that uniquely identifies the machine
         * @return The PID of the spawned process
         */
        map_target_pid_function pid;

        /**
         * Pointer to a function that constructs a future for each target
         *
         * @param data An arbitrary data structure
         * @param target A target from the target array
         * @param client_interface Executable to invoke to reach the target machine
         * @param target_key Key that uniquely identifies the machine
         * @return A future instance
         */
        map_target_future_function future;
    }
    map_target_function;

    /** Function that gets executed when a process completes for a target */
    union
    {
        /**
         * Pointer to a function that gets executed when a process completes for a target
         *
         * @param data An arbitrary data structure
         * @param target A target from the target array
         * @param target_key Key that uniquely identifies the machine
         * @param status Indicates whether the process terminated abnormally or not
         * @param result TRUE if the operation succeeded, else FALSE
         */
        complete_target_mapping_pid_function pid;

        /**
         * Pointer to a function that constructs a future when a process completes for a target
         *
         * @param data An arbitrary data structure
         * @param target A target from the target array
         * @param target_key Key that uniquely identifies the machine
         * @param future The future that has completed
         * @param status Indicates whether the process terminated abnormally or not
         */
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
 * @param target_property Global key that is used if no target property is defined by the target machine
 * @param interface Command-line tool that is invoked to reach the target machine
 * @param map_target Function that executes a process for each target
 * @param complete_target_mapping Function that gets executed when a process completes for a target
 * @param data Pointer to arbitrary data passed to the above functions
 * @return A PID iterator that can be used to traverse the targets
 */
ProcReact_PidIterator create_target_pid_iterator(GHashTable *targets_table, const gchar *target_property, gchar *interface, map_target_pid_function map_target, complete_target_mapping_pid_function complete_target_mapping, void *data);

/**
 * Creates a new future iterator that steps over each target and executes the
 * provided functions on start and completion.
 *
 * @param targets_table Hash table with targets
 * @param target_property Global key that is used if no target property is defined by the target machine
 * @param interface Command-line tool that is invoked to reach the target machine
 * @param map_target Function that executes a process for each target
 * @param complete_target_mapping Function that gets executed when a process completes for a target
 * @param data Pointer to arbitrary data passed to the above functions
 * @return A future iterator that can be used to traverse the targets
 */
ProcReact_FutureIterator create_target_future_iterator(GHashTable *targets_table, const gchar *target_property, gchar *interface, map_target_future_function map_target, complete_target_mapping_future_function complete_target_mapping, void *data);

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
int target_iterator_has_succeeded(const TargetIteratorData *target_iterator_data);

#endif
