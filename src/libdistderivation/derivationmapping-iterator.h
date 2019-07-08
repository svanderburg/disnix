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

#ifndef __DISNIX_DERIVATIONMAPPING_ITERATOR_H
#define __DISNIX_DERIVATIONMAPPING_ITERATOR_H
#include <procreact_pid_iterator.h>
#include <procreact_future_iterator.h>
#include "derivationmappingarray.h"

/** Pointer to a function that executes a process for each derivation item */
typedef pid_t (*map_derivation_item_pid_function) (void *data, DerivationItem *item, Interface *interface);
/** Pointer to a function that constructs a future for each derivation item */
typedef ProcReact_Future (*map_derivation_item_future_function) (void *data, DerivationItem *item, Interface *interface);

/** Pointer to a function that gets executed when a process completes for a derivation item */
typedef void (*complete_derivation_item_mapping_pid_function) (void *data, DerivationItem *item, ProcReact_Status status, int result);
/** Pointer to a function that gets executed when a future completes for a derivation item */
typedef void (*complete_derivation_item_mapping_future_function) (void *data, DerivationItem *item, ProcReact_Future *future, ProcReact_Status status);

/**
 * @brief Iterator that can be used to execute a process for each derivation item
 */
typedef struct
{
    /** Common properties for all model iterators */
    ModelIteratorData model_iterator_data;
    /** Array with derivation items */
    const GPtrArray *derivation_array;
    /** Hash table with interface items */
    GHashTable *interfaces_table;

    /** Function that maps over each item in the derivation array */
    union
    {
        /**
         * Pointer to a function that executes a process for each derivation item.
         *
         * @param data An arbitrary data structure
         * @param item A derivation item from the distributed derivation
         * @param interface The corresponding interface of the derivation item
         * @return The PID of the spawned process
         */
        map_derivation_item_pid_function pid;
        /**
         * Pointer to a function that constructs a future for each derivation item
         *
         * @param data An arbitrary data structure
         * @param item A derivation item from the distributed derivation
         * @param interface The corresponding interface of the derivation item
         * @return A future instance
         */
        map_derivation_item_future_function future;
    }
    map_derivation_item_function;

    /** Function that gets executed when a mapping function completes */
    union
    {
        /**
         * Pointer to a function that gets executed when a process completes for
         * a derivation item.
         *
         * @param data An arbitrary data structure
         * @param item A derivation item from the distributed derivation
         * @param status Indicates whether the process terminated abnormally or not
         * @param result TRUE if the operation succeeded, else FALSE
         */
        complete_derivation_item_mapping_pid_function pid;
        /**
         * Pointer to a function that gets executed when a future completes for
         * a derivation item.
         *
         * @param data An arbitrary data structure
         * @param item A derivation item from the distributed derivation
         * @param future The future that has completed
         * @param status Indicates whether the process terminated abnormally or not
         */
        complete_derivation_item_mapping_future_function future;
    }
    complete_derivation_item_mapping_function;

    /** Pointer to arbitrary data passed to the above functions */
    void *data;
}
DerivationIteratorData;

/**
 * Creates a new PID iterator that steps over each derivation item and interface
 * and executes the provided functions on start and completion.
 *
 * @param derivation_array Array with derivation items
 * @param interfaces_table Hash table with interfaces
 * @param map_derivation_item Pointer to a function that executes an operation for each derivation item
 * @param complete_derivation_item_mapping Pointer to a function that gets executed when a process completes for a derivation item
 * @param data Pointer to arbitrary data passed to the above functions
 * @return A PID iterator that can be used to traverse the derivation items
 */
ProcReact_PidIterator create_derivation_pid_iterator(const GPtrArray *derivation_array, GHashTable *interfaces_table, map_derivation_item_pid_function map_derivation_item, complete_derivation_item_mapping_pid_function complete_derivation_item_mapping, void *data);

/**
 * Creates a new future iterator that steps over each derivation item and
 * interface and executes the provided functions on start and completion.
 *
 * @param derivation_array Array with derivation items
 * @param interfaces_table Hash table with interfaces
 * @param map_derivation_item Pointer to a function that constructs a future for each derivation item
 * @param complete_derivation_item_mapping Pointer to a function that gets executed when a future completes for a derivation item
 * @param data Pointer to arbitrary data passed to the above functions
 * @return A future iterator that can be used to traverse the derivation items
 */
ProcReact_FutureIterator create_derivation_future_iterator(const GPtrArray *derivation_array, GHashTable *interfaces_table, map_derivation_item_future_function map_derivation_item, complete_derivation_item_mapping_future_function complete_derivation_item_mapping, void *data);

/**
 * Destroys all resources allocated with the provided derivation PID iterator
 *
 * @param iterator A derivation PID iterator instance
 */
void destroy_derivation_pid_iterator(ProcReact_PidIterator *iterator);

/**
 * Destroys all resources allocated with the provided derivation future iterator
 *
 * @param iterator A derivation future iterator instance
 */
void destroy_derivation_future_iterator(ProcReact_FutureIterator *iterator);

/**
 * Checks whether all iteration steps have succeeded.
 *
 * @param derivation_iterator_data Struct with properties that facilitate iteration over derivation items
 * @return TRUE if it indicates success, else FALSE
 */
int derivation_iterator_has_succeeded(const DerivationIteratorData *derivation_iterator_data);

#endif
