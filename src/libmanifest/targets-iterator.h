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
#include <procreact_pid_iterator.h>
#include "targets.h"

/** Pointer to a function that executes a process for each target */
typedef pid_t (*map_target_function) (void *data, Target *target);

/** Pointer to a function that gets executed when a process completes for a target */
typedef void (*complete_target_mapping_function) (void *data, Target *target, ProcReact_Status status, int result);

/**
 * @brief Iterator that can be used to execute a process for each target
 */
typedef struct
{
    /** Common properties for all model iterators */
    ModelIteratorData model_iterator_data;
    /** Array with targets */
    const GPtrArray *target_array;
    /**
     * Pointer to a function that executes a process for each target
     *
     * @param data An arbitrary data structure
     * @param target A target from the target array
     * @param client_interface Executable to invoke to reach the target machine
     * @param target_key Key that uniquely identifies the machine
     * @return The PID of the spawned process
     */
    map_target_function map_target;
    /**
     * Pointer to a function that gets executed when a process completes for a target
     *
     * @param data An arbitrary data structure
     * @param target A target from the target array
     * @param target_key Key that uniquely identifies the machine
     * @param status Indicates whether the process terminated abnormally or not
     * @param result TRUE if the operation succeeded, else FALSE
     */
    complete_target_mapping_function complete_target_mapping;
    /** Pointer to arbitrary data passed to the above functions */
    void *data;
}
TargetIteratorData;

/**
 * Creates a new PID iterator that steps over each target and executes the
 * provided functions on start and completion.
 *
 * @param target_array Array with targets
 * @param map_target Function that executes a process for each target
 * @param complete_target_mapping Function that gets executed when a process completes for a target
 * @param data Pointer to arbitrary data passed to the above functions
 * @return A PID iterator that can be used to traverse the targets
 */
ProcReact_PidIterator create_target_iterator(const GPtrArray *target_array, map_target_function map_target, complete_target_mapping_function complete_target_mapping, void *data);

/**
 * Destroys all resources allocated with a target PID iterator
 *
 * @param iterator A PID iterator instance
 */
void destroy_target_iterator(ProcReact_PidIterator *iterator);

/**
 * Checks whether all iteration steps have succeeded.
 *
 * @param iterator Struct with properties that facilitate iteration over targets
 * @return TRUE if it indicates success, else FALSE
 */
int target_iterator_has_succeeded(const ProcReact_PidIterator *iterator);

#endif
