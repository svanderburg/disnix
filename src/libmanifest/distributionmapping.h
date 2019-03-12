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

#ifndef __DISNIX_DISTRIBUTIONMAPPING_H
#define __DISNIX_DISTRIBUTIONMAPPING_H
#include <glib.h>
#include <libxml/parser.h>
#include <procreact_pid_iterator.h>
#include <modeliterator.h>
#include "targets.h"

/**
 * @brief Contains a mapping of a Nix profile to a disnix service target
 */
typedef struct
{
    /** Nix store path to the profile */
    gchar *profile;
    /** Address of a disnix service */
    gchar *target;
}
DistributionItem;

/** Pointer to a function that executes an operation for each distribution item */
typedef pid_t (*map_distribution_item_function) (void *data, DistributionItem *item, Target *target);

/** Pointer to a function that gets executed when a process completes for a distribution item */
typedef void (*complete_distribution_item_mapping_function) (void *data, DistributionItem *item, ProcReact_Status status, int result);

/**
 * @brief Iterator that can be used to execute a process for each distribution item
 */
typedef struct
{
    /** Common properties for all model iterators */
    ModelIteratorData model_iterator_data;
    /** Array with distribution items */
    const GPtrArray *distribution_array;
    /** Array with target items */
    const GPtrArray *target_array;
    
    /**
     * Pointer to a function that executes an operation for each distribution item
     *
     * @param data An arbitrary data structure
     * @param item A distribution item from the manifest
     * @param target The corresponding target machine of the distribution item
     * @return The PID of the spawned process
     */
    map_distribution_item_function map_distribution_item;
    
    /**
     * Pointer to a function that gets executed when a process completes for a distribution item
     *
     * @param data An arbitrary data structure
     * @param item A distribution item from the manifest
     * @param status Indicates whether the process terminated abnormally or not
     * @param result TRUE if the operation succeeded, else FALSE
     */
    complete_distribution_item_mapping_function complete_distribution_item_mapping;

    /** Pointer to arbitrary data passed to the above functions */
    void *data;
}
DistributionIteratorData;

/**
 * Creates a new array with distribution items from a manifest file.
 *
 * @param manifest_file Path to the manifest XML file
 * @return GPtrArray with DistributionItems
 */
//GPtrArray *generate_distribution_array(const gchar *manifest_file);
GPtrArray *parse_distribution(xmlNodePtr element);

/**
 * Deletes an array with distribution items.
 *
 * @param distribution_array Array with distribution items
 */
void delete_distribution_array(GPtrArray *distribution_array);

void print_distribution_array(const GPtrArray *distribution_array);

int check_distribution_array(const GPtrArray *distribution_array);

/**
 * Creates a new iterator that steps over each distribution item and target and
 * executes the provided functions on start and completion.
 *
 * @param distribution_array Array with distribution items
 * @param target_array Array with target items
 * @param map_distribution_item Pointer to a function that executes an operation for each distribution item
 * @param complete_distribution_item_mapping Pointer to a function that gets executed when a process completes for a distribution item
 * @param data Pointer to arbitrary data passed to the above functions
 * @return A PID iterator that can be used to traverse the distribution items
 */
ProcReact_PidIterator create_distribution_iterator(const GPtrArray *distribution_array, const GPtrArray *target_array, map_distribution_item_function map_distribution_item, complete_distribution_item_mapping_function complete_distribution_item_mapping, void *data);

/**
 * Destroys the resources attached to the given distribution iterator.
 *
 * @param iterator Pid iterator constructed with create_distribution_iterator()
 */
void destroy_distribution_iterator(ProcReact_PidIterator *iterator);

/**
 * Returns the success status of the overall iteration process.
 *
 * @return TRUE if all the operations of the iterator have succeeded else FALSE.
 */
int distribution_iterator_has_succeeded(const ProcReact_PidIterator *iterator);

#endif
