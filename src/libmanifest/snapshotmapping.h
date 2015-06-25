/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2015  Sander van der Burg
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

#ifndef __DISNIX_SNAPSHOTMAPPING_H
#define __DISNIX_SNAPSHOTMAPPING_H
#include <glib.h>

/**
 * @brief Contains the values that constitute a key uniquely referring to a snapshot mapping.
 */
typedef struct
{
    /** Name of the mutable component */
    gchar *component;
    
    /** Container in which the mutable component is deployed */
    gchar *container;
    
    /** Target property referring to the target machine to which the service is deployed */
    gchar *target;
}
SnapshotMappingKey;

/**
 * @brief Contains all properties to snapshot state on a specific machine.
 * This struct maps (component,container,target) -> (transferred)
 */
typedef struct
{
    /** Name of the mutable component */
    gchar *component;
    
    /** Container in which the mutable component is deployed */
    gchar *container;
    
    /** Target property referring to the target machine to which the service is deployed */
    gchar *target;
    
    /** Full Nix store path to the corresponding service */
    gchar *service;
    
    /** Indicates whether the snapshot has been transferred or not */
    gboolean transferred;
}
SnapshotMapping;

/**
 * Creates an array with activation mappings from a manifest XML file.
 *
 * @param manifest_file Path to the manifest XML file
 * @return GPtrArray containing activation mappings
 */
GPtrArray *create_snapshots_array(const gchar *manifest_file);

/**
 * Deletes an array with snapshot mappings including its contents.
 *
 * @param snapshots_array Snapshots array to delete
 */
void delete_snapshots_array(GPtrArray *snapshots_array);

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
GPtrArray *subtract_snapshot_mappings(GPtrArray *snapshots_array1, GPtrArray *snapshots_array2);

#endif
