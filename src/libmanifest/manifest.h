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

#ifndef __DISNIX_MANIFEST_H
#define __DISNIX_MANIFEST_H

#include <stdio.h>
#include <glib.h>

#define MANIFEST_PROFILES_FLAG 0x1
#define MANIFEST_SERVICE_MAPPINGS_FLAG 0x2
#define MANIFEST_SNAPSHOT_MAPPINGS_FLAG 0x4
#define MANIFEST_INFRASTRUCTURE_FLAG 0x8
#define MANIFEST_ALL_FLAGS (MANIFEST_PROFILES_FLAG | MANIFEST_SERVICE_MAPPINGS_FLAG | MANIFEST_SNAPSHOT_MAPPINGS_FLAG | MANIFEST_INFRASTRUCTURE_FLAG)

/**
 * @brief Contains all properties of a manifest file that is used for distribution and activation
 *
 * A deployment manifest consists of an array mapping intra-dependency closures
 * to targets, an array of services that need to be activated on a specific
 * target and array of available target machines
 */
typedef struct
{
    /** Hash table mapping targets to Nix profiles containing intra-dependency closures */
    GHashTable *profile_mapping_table;

    /** Hash table containing the properties of all services */
    GHashTable *services_table;

    /** Array of services to be activated on a target machine */
    GPtrArray *service_mapping_array;

    /** Array of snapshots to be tranferred to a target machine */
    GPtrArray *snapshot_mapping_array;

    /** Hash table of the available target machines */
    GHashTable *targets_table;

    /** Indicates whether the manifest has the correct version */
    int correct_version;
}
Manifest;

/**
 * Composes a manifest struct from a manifest file. The resulting manifest can
 * be freed with delete_manifest() when it is not needed anymore.
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
 * Checks whether all properties of a manifest are valid.
 *
 * @param manifest Manifest struct instance
 * @return TRUE if the manifest is valid, else FALSE
 */
int check_manifest(const Manifest *manifest);

/**
 * Checks whether two manifest struct instances are identical.
 *
 * @param manifest1 Manifest struct instance
 * @param manifest2 Manifest struct instance
 * @return TRUE if both instances are equal, else FALSE
 */
int compare_manifests(const Manifest *manifest1, const Manifest *manifest2);

/**
 * Prints a Nix expression representation of a manifest.
 *
 * @param file File descriptor to write to
 * @param manifest Manifest struct instance
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_manifest_nix(FILE *file, const Manifest *manifest, const int indent_level, void *userdata);

/**
 * Prints an XML representation of a manifest.
 *
 * @param file File descriptor to write to
 * @param manifest Manifest struct instance
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_manifest_xml(FILE *file, const Manifest *manifest, const int indent_level, const char *type_property_name, void *userdata);

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
 * Opens the provided manifest or (if the specified manifest file is NULL)
 * attempts to open the manifest of the last deployed configuration.
 *
 * @param manifest_file Manifest file to open
 * @param coordinator_profile_path Path to the coordinator profile or NULL to consult the default profile path
 * @param profile Name of the Disnix profile that identifies the deployment (typically: default)
 * @param flags Flags indicating which portions of the manifest should be parsed
 * @param container_filter Name of the container to filter on, or NULL to parse all containers
 * @param component_filter Name of the component to filter on, or NULL to parse all components
 * @return The provided manifest, previous manifest, or NULL in case of an error
 */
Manifest *open_provided_or_previous_manifest_file(const gchar *manifest_file, const gchar *coordinator_profile_path, gchar *profile, const unsigned int flags, const gchar *container_filter, const gchar *component_filter);

/**
 * Opens the old manifest file, or if it is NULL the previous manifest in the
 * coordinator profile.
 *
 * @param old_manifest Path to the previous manifest file
 * @param coordinator_profile_path Path to the coordinator profile or NULL to consult the default profile path
 * @param profile Name of the Disnix profile that identifies the deployment (typically: default)
 */
gchar *determine_manifest_to_open(const gchar *old_manifest, const gchar *coordinator_profile_path, gchar *profile);

/**
 * Opens the previous provided manifest file
 *
 * @param manifest_file Manifest file to open
 * @param flags Flags indicating which portions of the manifest should be parsed
 * @param container_filter Name of the container to filter on, or NULL to parse all containers
 * @param component_filter Name of the component to filter on, or NULL to parse all components
 */
Manifest *open_previous_manifest(const gchar *manifest_file, const unsigned int flags, const gchar *container_filter, const gchar *component_filter);

#endif
