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

#ifndef __DISNIX_PROFILEMANIFESTTARGET_H
#define __DISNIX_PROFILEMANIFESTTARGET_H

#include <glib.h>
#include <procreact_pid_iterator.h>
#include <modeliterator.h>
#include "profilemanifest.h"

/**
 * @brief Contains all runtime properties of a target machine's deployment configuration
 */
typedef struct
{
    /** Path to the Nix store path of the profile containing the intra-dependency closures of all deployed services */
    gchar *profile;
    /** An array of profile manifest entries */
    ProfileManifest *profile_manifest;
}
ProfileManifestTarget;

/** Pointer to a function that executes an operation for each profile manifest target */
typedef pid_t (*map_profilemanifesttarget_item_function) (void *data, gchar *target_key, ProfileManifestTarget *profile_manifest_target);

/** Pointer to a function that gets executed when a process completes for a profile manifest target */
typedef void (*complete_profilemanifesttarget_item_mapping_function) (void *data, ProfileManifestTarget *profile_manifest_target, ProcReact_Status status, int result);

/**
 * @brief Iterator that can be used to execute a process for each profile manifest target
 */
typedef struct
{
    /** Common properties for all model iterators */
    ModelIteratorData model_iterator_data;
    GHashTableIter iter;
    /** Hash table with profile manifest targets */
    GHashTable *profile_manifest_target_table;

    /**
     * Pointer to a function that executes an operation for each profile manifest target
     *
     * @param data An arbitrary data structure
     * @param profile_manifest_target A profile manifest target from the array
     * @return The PID of the spawned process
     */
    map_profilemanifesttarget_item_function map_profilemanifesttarget_item;

    /**
     * Pointer to a function that gets executed when a process completes for a profile manifest target
     *
     * @param data An arbitrary data structure
     * @param profile_manifest_target A profile manifest target from the array
     * @param status Indicates whether the process terminated abnormally or not
     * @param result TRUE if the operation succeeded, else FALSE
     */
    complete_profilemanifesttarget_item_mapping_function complete_profilemanifesttarget_item_mapping;

    /** Pointer to arbitrary data passed to the above functions */
    void *data;
}
ProfileManifestTargetIteratorData;

/**
 * Parses the manifest from the derivation value and construct arrays of profile
 * manifest entries from it.
 *
 * @param profile_manifest_target A profile manifest target
 */
void parse_profile_manifest_target(ProfileManifestTarget *profile_manifest_target);

/**
 * Deletes the given profile manifest target array and its properties from
 * memory.
 *
 * @param profile_manifest_target_array Array of profile manifest targets
 */
void delete_profile_manifest_target_table(GHashTable *profile_manifest_target_table);

/**
 * Prints for each target the deployed services.
 *
 * @param profile_manifest_target_array Array of profile manifest targets
 */
void print_services_in_profile_manifest_target_table(GHashTable *profile_manifest_target_table);

/**
 * Prints for each target the deployed services per container.
 *
 * @param profile_manifest_target_array Array of profile manifest targets
 */
void print_services_per_container_in_profile_manifest_target_table(GHashTable *profile_manifest_target_table);

/**
 * Prints a Nix expression containing profile mappings and service mappings
 * derived from the profile manifests on all machines. This expression can be
 * used to reconstruct the deployment manifest on the coordinator.
 *
 * @param profile_manifest_target_array Array of profile manifest targets
 */
void print_profile_manifest_target_table_nix(GHashTable *profile_manifest_target_table, void *userdata);

void print_profile_manifest_target_table_xml(GHashTable *profile_manifest_target_table, void *userdata);

/**
 * Creates a new iterator that steps over each profile manifest target and
 * executes the provided functions on start and completion.
 *
 * @param profile_manifest_target_array Array of profile manifest targets
 * @param map_profilemanifesttarget_item Pointer to a function that executes an operation for each profile manifest target
 * @param complete_profilemanifesttarget_item_mapping Pointer to a function that gets executed when a process completes for a profile manifest target
 * @param data Pointer to arbitrary data passed to the above functions
 * @return A PID iterator that can be used to traverse the distribution items
 */
ProcReact_PidIterator create_profile_manifest_target_iterator(GHashTable *profile_manifest_target_table, map_profilemanifesttarget_item_function map_profilemanifesttarget_item, complete_profilemanifesttarget_item_mapping_function complete_profilemanifesttarget_item_mapping, void *data);

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
