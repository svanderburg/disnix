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

#ifndef __DISNIX_MANIFEST_H
#define __DISNIX_MANIFEST_H
#include <glib.h>

#define MANIFEST_DISTRIBUTION_FLAG 0x1
#define MANIFEST_ACTIVATION_FLAG 0x2
#define MANIFEST_SNAPSHOT_FLAG 0x4
#define MANIFEST_ALL_FLAGS (MANIFEST_DISTRIBUTION_FLAG | MANIFEST_ACTIVATION_FLAG | MANIFEST_SNAPSHOT_FLAG)

/**
 * @brief Contains all properties of a manifest file that is used for distribution and activation
 *
 * A deployment manifest consists of an array mapping intra-dependency closures
 * to targets, an array of services that need to be activated on a specific
 * target and array of available target machines
 */
typedef struct
{
    /** Array mapping intra-dependency closures to machines */
    GPtrArray *distribution_array;
    
    /** Array of services to be activated on a target machine */
    GPtrArray *activation_array;
    
    /** Array of snapshots to be tranferred to a target machine */
    GPtrArray *snapshots_array;
    
    /** Array containing the available target machines */
    GPtrArray *target_array;
}
Manifest;

/**
 * Composes a manifest struct from a manifest file.
 *
 * @param manifest_file Manifest file to open
 * @param flags Flags indicating which portions of the manifest should be parsed
 * @param container_filter Name of the container to filter on, or NULL to parse all containers
 * @param component_filter Name of the component to filter on, or NULL to parse all components
 * @return A manifest struct or NULL if an error occurred
 */
Manifest *create_manifest(const gchar *manifest_file, const unsigned int flags, const gchar *container_filter, const gchar *component_filter);

/**
 * Deletes a manifest struct from heap memory.
 *
 * @param manifest Manifest to delete
 */
void delete_manifest(Manifest *manifest);

/**
 * Determines the path of the last generation of the coordinator profile which
 * corresponds to the manifest of the last deployed configuration.
 *
 * @param coordinator_profile_path Path to the coordinator profile or NULL to consult the default profile path
 * @param profile Name of the Disnix profile that identifies the deployment (typically: default)
 * @return The path to the latest coordinator profile generation or NULL if no previous deployment exists.
 *   The resulting string is allocated on the heap and should eventually be freed with g_free()
 */
gchar *determine_previous_manifest_file(const gchar *coordinator_profile_path, const gchar *profile);

/**
 * Opens the provided manifest or (if NULL) it attempts to open the manifest of
 * the last deployed configuration.
 *
 * @param manifest_file Manifest file to open
 * @param coordinator_profile_path Path to the coordinator profile or NULL to consult the default profile path
 * @param profile Name of the Disnix profile that identifies the deployment (typically: default)
 * @param flags Flags indicating which portions of the manifest should be parsed
 * @param container Name of the container to filter on, or NULL to parse all containers
 * @param component Name of the component to filter on, or NULL to parse all components
 * @return The provided manifest, previous manifest, or NULL in case of an error
 */
Manifest *open_provided_or_previous_manifest_file(const gchar *manifest_file, const gchar *coordinator_profile_path, gchar *profile, const unsigned int flags, const gchar *container, const gchar *component);

gchar *determine_manifest_to_open(const gchar *old_manifest, const gchar *coordinator_profile_path, gchar *profile);

Manifest *open_previous_manifest(const gchar *manifest_file, const unsigned int flags, const gchar *container, const gchar *component);

#endif
