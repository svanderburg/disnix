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
#include "snapshotmapping.h"

/**
 * Creates an array with activation mappings from the corresponding sub section
 * in an XML document.
 *
 * @param element XML root element of the sub section defining the mappings
 * @param container_filter Name of the container to filter on, or NULL to parse all containers
 * @param component_filter Name of the component to filter on, or NULL to parse all components
 * @param userdata Arbitrary user data that is propagated to all parse functions
 * @return GPtrArray containing activation mappings
 */
GPtrArray *parse_snapshot_mapping_array(xmlNodePtr element, const gchar *container_filter, const gchar *component_filter, void *userdata);

/**
 * Deletes an array with snapshot mappings including its contents.
 *
 * @param snapshot_mapping_array Snapshots array to delete
 */
void delete_snapshot_mapping_array(GPtrArray *snapshot_mapping_array);

/**
 * Checks whether an array of snapshot mappings is valid.
 *
 * @param snapshot_mapping_array A GPtrArray with snapshot mappings
 * @return TRUE when the content is valid, else FALSE
 */
int check_snapshot_mapping_array(const GPtrArray *snapshot_mapping_array);

/**
 * Compares two snapshot mapping arrays and checks whether they have the same content
 *
 * @param snapshot_mapping_array1 A GPtrArray with snapshot mappings
 * @param snapshot_mapping_array2 A GPtrArray with snapshot mappings
 * @return TRUE if the arrays have equal content, else FALSE
 */
int compare_snapshot_mapping_arrays(const GPtrArray *snapshot_mapping_array1, const GPtrArray *snapshot_mapping_array2);

/**
 * Prints a Nix expression representation of a snapshot mapping array.
 *
 * @param file File descriptor to write to
 * @param snapshot_mapping_array A GPtrArray with snapshot mappings
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_snapshot_mapping_array_nix(FILE *file, const GPtrArray *snapshot_mapping_array, const int indent_level, void *userdata);

/**
 * Prints an XML representation of a snapshot mapping array.
 *
 * @param file File descriptor to write to
 * @param snapshot_mapping_array A GPtrArray with snapshot mappings
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_snapshot_mapping_array_xml(FILE *file, const GPtrArray *snapshot_mapping_array, const int indent_level, const char *type_property_name, void *userdata);

/**
 * Returns the snapshots mapping with the given key in the snapshots array.
 *
 * @param snapshot_mapping_array Snapshots array
 * @param key Key of the snapshots mapping to find
 * @return The snapshot mapping with the specified key, or NULL if it cannot be found
 */
SnapshotMapping *find_snapshot_mapping(const GPtrArray *snapshot_mapping_array, const SnapshotMappingKey *key);

/**
 * Subtract the snapshots from array1 that are in array2.
 *
 * @param snapshot_mapping_array1 Array to substract from
 * @param snapshot_mapping_array2 Array to substract
 * @return An array with snapshot mappings with elements from array1 that are not in array2
 */
GPtrArray *subtract_snapshot_mappings(const GPtrArray *snapshot_mapping_array1, const GPtrArray *snapshot_mapping_array2);

/**
 * Finds all snapshot mappings that map to a specific target.
 *
 * @param snapshot_mapping_array Snapshots array
 * @param target Key that identifies a target machine
 * @return An array with snapshot mappings linked to the given target
 */
GPtrArray *find_snapshot_mappings_per_target(const GPtrArray *snapshot_mapping_array, const gchar *target);

/**
 * Resets the transferred status on all items in the snapshots array
 *
 * @param snapshot_mapping_array Snapshots array
 */
void reset_snapshot_items_transferred_status(GPtrArray *snapshot_mapping_array);

#endif
