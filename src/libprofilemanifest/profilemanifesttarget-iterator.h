/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2019  Sander van der Burg
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

#ifndef __DISNIX_PROFILEMANIFESTTARGET_ITERATOR_H
#define __DISNIX_PROFILEMANIFESTTARGET_ITERATOR_H
#include <glib.h>
#include <procreact_pid_iterator.h>
#include <modeliterator.h>
#include <target.h>
#include "profilemanifesttargettable.h"

/**
 * Pointer to a function that executes an operation for each profile manifest target
 *
 * @param data An arbitrary data structure
 * @param target_name Name that uniquely identifies the machine
 * @param target A target from the targets table
 * @param profile_manifest_target A profile manifest target from the array
 * @return The PID of the spawned process
 */
typedef pid_t (*map_profilemanifesttarget_function) (void *data, gchar *target_name, ProfileManifestTarget *profile_manifest_target, Target *target);

/**
 * Pointer to a function that gets executed when a process completes for a profile manifest target
 *
 * @param data An arbitrary data structure
 * @param target_name Name that uniquely identifies the machine
 * @param target A target from the targets table
 * @param profile_manifest_target A profile manifest target from the array
 * @param status Indicates whether the process terminated abnormally or not
 * @param result TRUE if the operation succeeded, else FALSE
 */
typedef void (*complete_profilemanifesttarget_function) (void *data, gchar *target_name, ProfileManifestTarget *profile_manifest_target, Target *target, ProcReact_Status status, int result);

/**
 * @brief Iterator that can be used to execute a process for each profile manifest target
 */
typedef struct
{
    /** Common properties for all model iterators */
    ModelIteratorData model_iterator_data;
    /** Iterator for the hash table with profile manifest targets */
    GHashTableIter iter;
    /** Hash table with profile manifest targets */
    GHashTable *profile_manifest_target_table;

    /** Pointer to a function that executes an operation for each profile manifest target */
    map_profilemanifesttarget_function map_profilemanifesttarget;
    /** Pointer to a function that gets executed when a process completes for a profile manifest target */
    complete_profilemanifesttarget_function complete_profilemanifesttarget;

    /** Hash table with targets */
    GHashTable *targets_table;

    /** Pointer to arbitrary data passed to the above functions */
    void *data;
}
ProfileManifestTargetIteratorData;

/**
 * Creates a new iterator that steps over each profile manifest target and
 * executes the provided functions on start and completion.
 *
 * @param profile_manifest_target_table Hash table of profile manifest targets
 * @param map_profilemanifesttarget Pointer to a function that executes an operation for each profile manifest target
 * @param complete_profilemanifesttarget Pointer to a function that gets executed when a process completes for a profile manifest target
 * @param data Pointer to arbitrary data passed to the above functions
 * @param targets_table Hash table with targets
 * @return A PID iterator that can be used to traverse the profile manifest targets
 */
ProcReact_PidIterator create_profile_manifest_target_iterator(GHashTable *profile_manifest_target_table, map_profilemanifesttarget_function map_profilemanifesttarget, complete_profilemanifesttarget_function complete_profilemanifesttarget, GHashTable *targets_table, void *data);

/**
 * Destroys the resources attached to the given profile manifest target iterator.
 *
 * @param iterator Pid iterator constructed with create_profile_manifest_target_iterator()
 */
void destroy_profile_manifest_target_iterator(ProcReact_PidIterator *iterator);

/**
 * Returns the success status of the overall iteration process.
 *
 * @return TRUE if all the operations of the iterator have succeeded else FALSE.
 */
int profile_manifest_target_iterator_has_succeeded(const ProcReact_PidIterator *iterator);

#endif
