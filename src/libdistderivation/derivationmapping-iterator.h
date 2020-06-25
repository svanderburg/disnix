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

#ifndef __DISNIX_DERIVATIONMAPPING_ITERATOR_H
#define __DISNIX_DERIVATIONMAPPING_ITERATOR_H
#include <procreact_pid_iterator.h>
#include <procreact_future_iterator.h>
#include <modeliterator.h>
#include "derivationmappingarray.h"
#include "interfacestable.h"

/**
 * Pointer to a function that executes a process for each derivation mapping.
 *
 * @param data An arbitrary data structure
 * @param mapping A derivation mapping from the distributed derivation
 * @param interface The corresponding interface of the derivation mapping
 * @return The PID of the spawned process
 */
typedef pid_t (*map_derivation_mapping_pid_function) (void *data, DerivationMapping *mapping, Interface *interface);

/**
 * Pointer to a function that constructs a future for each derivation mapping
 *
 * @param data An arbitrary data structure
 * @param mapping A derivation mapping from the distributed derivation
 * @param interface The corresponding interface of the derivation mapping
 * @return A future instance
 */
typedef ProcReact_Future (*map_derivation_mapping_future_function) (void *data, DerivationMapping *mapping, Interface *interface);

/**
 * Pointer to a function that gets executed when a process completes for
 * a derivation mapping.
 *
 * @param data An arbitrary data structure
 * @param mapping A derivation mapping from the distributed derivation
 * @param status Indicates whether the process terminated abnormally or not
 * @param result TRUE if the operation succeeded, else FALSE
 */
typedef void (*complete_derivation_mapping_pid_function) (void *data, DerivationMapping *mapping, ProcReact_Status status, ProcReact_bool result);

/**
 * Pointer to a function that gets executed when a future completes for
 * a derivation mapping.
 *
 * @param data An arbitrary data structure
 * @param mapping A derivation mapping from the distributed derivation
 * @param future The future that has completed
 * @param status Indicates whether the process terminated abnormally or not
 */
typedef void (*complete_derivation_mapping_future_function) (void *data, DerivationMapping *mapping, ProcReact_Future *future, ProcReact_Status status);

/**
 * @brief Iterator that can be used to execute a process for each derivation mapping
 */
typedef struct
{
    /** Common properties for all model iterators */
    ModelIteratorData model_iterator_data;
    /** Array with derivation mappings */
    const GPtrArray *derivation_mapping_array;
    /** Hash table with interfaces */
    GHashTable *interfaces_table;

    /** Function that maps over each mapping in the derivation mapping array */
    union
    {
        /** Pointer to a function that executes a process for each derivation mapping */
        map_derivation_mapping_pid_function pid;
        /** Pointer to a function that constructs a future for each derivation mapping */
        map_derivation_mapping_future_function future;
    }
    map_derivation_mapping_function;

    /** Function that gets executed when a mapping function completes */
    union
    {
        /** Pointer to a function that gets executed when a process completes for a derivation mapping */
        complete_derivation_mapping_pid_function pid;
        /** Pointer to a function that gets executed when a future completes for a derivation mapping */
        complete_derivation_mapping_future_function future;
    }
    complete_derivation_mapping_function;

    /** Pointer to arbitrary data passed to the above functions */
    void *data;
}
DerivationMappingIteratorData;

/**
 * Creates a new PID iterator that steps over each derivation mapping and interface
 * and executes the provided functions on start and completion.
 *
 * @param derivation_mapping_array Array with derivation mappings
 * @param interfaces_table Hash table with interfaces
 * @param map_derivation_mapping Pointer to a function that executes an operation for each derivation mapping
 * @param complete_derivation_mapping Pointer to a function that gets executed when a process completes for a derivation mapping
 * @param data Pointer to arbitrary data passed to the above functions
 * @return A PID iterator that can be used to traverse the derivation mappings
 */
ProcReact_PidIterator create_derivation_mapping_pid_iterator(const GPtrArray *derivation_mapping_array, GHashTable *interfaces_table, map_derivation_mapping_pid_function map_derivation_mapping, complete_derivation_mapping_pid_function complete_derivation_mapping, void *data);

/**
 * Creates a new future iterator that steps over each derivation mapping and
 * interface and executes the provided functions on start and completion.
 *
 * @param derivation_mapping_array Array with derivation mappings
 * @param interfaces_table Hash table with interfaces
 * @param map_derivation_mapping Pointer to a function that constructs a future for each derivation mapping
 * @param complete_derivation_mapping Pointer to a function that gets executed when a future completes for a derivation mapping
 * @param data Pointer to arbitrary data passed to the above functions
 * @return A future iterator that can be used to traverse the derivation mappings
 */
ProcReact_FutureIterator create_derivation_mapping_future_iterator(const GPtrArray *derivation_mapping_array, GHashTable *interfaces_table, map_derivation_mapping_future_function map_derivation_mapping, complete_derivation_mapping_future_function complete_derivation_mapping, void *data);

/**
 * Destroys all resources allocated with the provided derivation PID iterator
 *
 * @param iterator A derivation PID iterator instance
 */
void destroy_derivation_mapping_pid_iterator(ProcReact_PidIterator *iterator);

/**
 * Destroys all resources allocated with the provided derivation future iterator
 *
 * @param iterator A derivation future iterator instance
 */
void destroy_derivation_mapping_future_iterator(ProcReact_FutureIterator *iterator);

/**
 * Checks whether all iteration steps have succeeded.
 *
 * @param derivation_mapping_iterator_data Struct with properties that facilitate iteration over derivation mappings
 * @return TRUE if it indicates success, else FALSE
 */
ProcReact_bool derivation_mapping_iterator_has_succeeded(const DerivationMappingIteratorData *derivation_mapping_iterator_data);

#endif
