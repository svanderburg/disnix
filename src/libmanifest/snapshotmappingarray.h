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

#ifndef __DISNIX_SNAPSHOTMAPPINGARRAY_H
#define __DISNIX_SNAPSHOTMAPPINGARRAY_H
#include <glib.h>
#include <libxml/parser.h>
#include <procreact_pid.h>
#include <targetstable.h>
#include "servicestable.h"

/**
 * @brief Contains the values that constitute a key uniquely referring to a snapshot mapping.
 */
typedef struct
{
    /** Name of the mutable component */
    xmlChar *component;

    /** Container in which the mutable component is deployed */
    xmlChar *container;

    /** Target property referring to the target machine to which the service is deployed */
    xmlChar *target;
}
SnapshotMappingKey;

/**
 * @brief Contains all properties to snapshot state on a specific machine.
 * This struct maps (component,container,target) -> (transferred)
 */
typedef struct
{
    /** Name of the mutable component */
    xmlChar *component;

    /** Container in which the mutable component is deployed */
    xmlChar *container;

    /** Target property referring to the target machine to which the service is deployed */
    xmlChar *target;

    /** Hash code that uniquely defines a service */
    xmlChar *service;

    /** Indicates whether the snapshot has been transferred or not */
    gboolean transferred;
}
SnapshotMapping;

/**
 * Function that spawns a process for a snapshot mapping.
 *
 * @param mapping A snapshot mapping from a snapshots array
 * @param target Target machine to which the snapshot is mapped
 * @param arguments Arguments passed to the client interface
 * @param arguments_length Length of the arguments array
 * @return PID of the spawned process
 */
typedef pid_t (*map_snapshot_item_function) (SnapshotMapping *mapping, ManifestService *service, Target *target, gchar **arguments, unsigned int arguments_length);

/**
 * Function that gets executed when a mapping function completes.
 *
 * @param mapping A snapshot mapping from a snapshots array
 * @param status Indicates whether the process terminated abnormally or not
 * @param result TRUE if the mapping operation succeeded, else FALSE
 */
typedef void (*complete_snapshot_item_mapping_function) (SnapshotMapping *mapping, Target *target, ProcReact_Status status, int result);

gint compare_snapshot_mapping_keys(const SnapshotMappingKey **l, const SnapshotMappingKey **r);

/**
 * Creates an array with activation mappings from the corresponding sub section
 * in an XML document.
 *
 * @param element XML root element of the sub section defining the mappings
 * @param container_filter Name of the container to filter on, or NULL to parse all containers
 * @param component_filter Name of the component to filter on, or NULL to parse all components
 * @return GPtrArray containing activation mappings
 */
GPtrArray *parse_snapshot_mapping_array(xmlNodePtr element, const gchar *container_filter, const gchar *component_filter, void *userdata);

/**
 * Deletes an array with snapshot mappings including its contents.
 *
 * @param snapshots_array Snapshots array to delete
 */
void delete_snapshot_mapping_array(GPtrArray *snapshots_array);

int check_snapshot_mapping_array(const GPtrArray *snapshots_array);

int compare_snapshot_mapping_arrays(const GPtrArray *snapshot_mapping_array1, const GPtrArray *snapshot_mapping_array2);

/**
 * Returns the snapshots mapping with the given key in the snapshots array.
 *
 * @param snapshots_array Snapshots array
 * @param key Key of the snapshots mapping to find
 * @return The snapshot mapping with the specified key, or NULL if it cannot be found
 */
SnapshotMapping *find_snapshot_mapping(const GPtrArray *snapshots_array, const SnapshotMappingKey *key);

/**
 * Subtract the snapshots from array1 that are in array2.
 *
 * @param snapshots_array1 Array to substract from
 * @param snapshots_array2 Array to substract
 * @return An array with snapshot mappings with elements from array1 that are not in array2
 */
GPtrArray *subtract_snapshot_mappings(const GPtrArray *snapshots_array1, const GPtrArray *snapshots_array2);

/**
 * Finds all snapshot mappings that map to a specific target.
 *
 * @param snapshots_array Snapshots array
 * @param target Key that identifies a target machine
 * @return An array with snapshot mappings linked to the given target
 */
GPtrArray *find_snapshot_mappings_per_target(const GPtrArray *snapshots_array, const gchar *target);

/**
 * Maps over each snapshot mapping, asynchronously executes a function for each
 * item and ensures that for each machine only the allowed number of processes
 * are executed concurrently.
 *
 * @param snapshots_array Snapshots array
 * @param targets_table Hash table of targets
 * @param map_snapshot_item Function that gets executed for each snapshot item
 * @param complete_snapshot_item_mapping Function that gets executed when a mapping function completes
 * @return TRUE if all mappings were successfully executed, else FALSE
 */
int map_snapshot_items(const GPtrArray *snapshots_array, GHashTable *services_table, GHashTable *targets_table, map_snapshot_item_function map_snapshot_item, complete_snapshot_item_mapping_function complete_snapshot_item_mapping);

/**
 * Resets the transferred status on all items in the snapshots array
 *
 * @param snapshots_array Snapshots array
 */
void reset_snapshot_items_transferred_status(GPtrArray *snapshots_array);

void print_snapshot_mapping_array_nix(FILE *file, const void *value, const int indent_level, void *userdata);

#endif
