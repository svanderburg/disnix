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

#ifndef __DISNIX_PROFILEMAPPING_ITERATOR_H
#define __DISNIX_PROFILEMAPPING_ITERATOR_H

#include <procreact_pid_iterator.h>
#include <modeliterator.h>
#include "profilemappingtable.h"

/**
 * Pointer to a function that executes an operation for each profile mapping
 *
 * @param data An arbitrary data structure
 * @param target_name Name of the target in the table
 * @param profile_path Path to the Nix profile
 * @param target The corresponding target machine of the profile mapping
 * @return The PID of the spawned process
 */
typedef pid_t (*map_profile_mapping_function) (void *data, gchar *target_name, xmlChar *profile_path, Target *target);

/**
 * Pointer to a function that gets executed when a process completes for a profile mapping
 *
 * @param data An arbitrary data structure
 * @param target_name Name of the target in the table
 * @param profile_path Path to the Nix profile
 * @param target The corresponding target machine of the profile mapping
 * @param status Indicates whether the process terminated abnormally or not
 * @param result TRUE if the operation succeeded, else FALSE
 */
typedef void (*complete_map_profile_mapping_function) (void *data, gchar *target_name, xmlChar *profile_path, Target *target, ProcReact_Status status, ProcReact_bool result);

/**
 * @brief Iterator that can be used to execute a process for each profile mapping
 */
typedef struct
{
    /** Common properties for all model iterators */
    ModelIteratorData model_iterator_data;
    /** Hash table iterator */
    GHashTableIter iter;
    /** Hash table with profile mappings */
    GHashTable *profile_mapping_table;
    /** Hash table with target items */
    GHashTable *targets_table;

    /** Pointer to a function that executes an operation for each profile mapping */
    map_profile_mapping_function map_profile_mapping;
    /** Pointer to a function that gets executed when a process completes for a profile mapping */
    complete_map_profile_mapping_function complete_map_profile_mapping;

    /** Pointer to arbitrary data passed to the above functions */
    void *data;
}
ProfileMappingIteratorData;

/**
 * Creates a new iterator that steps over each profile mapping and
 * executes the provided functions on start and completion.
 *
 * @param profile_mapping_table Hash table with profile mappings
 * @param targets_table Hash table of targets
 * @param map_profile_mapping Pointer to a function that executes an operation for each distribution item
 * @param complete_map_profile_mapping Pointer to a function that gets executed when a process completes for a profile mapping
 * @param data Pointer to arbitrary data passed to the above functions
 * @return A PID iterator that can be used to traverse the profile mappings
 */
ProcReact_PidIterator create_profile_mapping_iterator(GHashTable *profile_mapping_table, GHashTable *targets_table, map_profile_mapping_function map_profile_mapping, complete_map_profile_mapping_function complete_map_profile_mapping, void *data);

/**
 * Destroys the resources attached to the given profile mapping iterator.
 *
 * @param iterator Pid iterator constructed with create_profile_mapping_iterator()
 */
void destroy_profile_mapping_iterator(ProcReact_PidIterator *iterator);

/**
 * Returns the success status of the overall iteration process.
 *
 * @return TRUE if all the operations of the iterator have succeeded else FALSE.
 */
ProcReact_bool profile_mapping_iterator_has_succeeded(const ProcReact_PidIterator *iterator);

#endif
