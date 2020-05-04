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

#ifndef __DISNIX_OLDMANIFEST_H
#define __DISNIX_OLDMANIFEST_H
#include <glib.h>

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
}
OldManifest;

/**
 * Composes a manifest struct from a manifest file.
 *
 * @param manifest_file Manifest file to open
 * @return A manifest struct or NULL if an error occurred
 */
OldManifest *create_old_manifest(const gchar *manifest_file);

/**
 * Deletes a manifest struct from heap memory.
 *
 * @param manifest Manifest to delete
 */
void delete_old_manifest(OldManifest *manifest);

int check_old_manifest(const OldManifest *manifest);

/**
 * Opens the provided manifest or (if NULL) it attempts to open the manifest of
 * the last deployed configuration.
 *
 * @param manifest_file Manifest file to open
 * @param coordinator_profile_path Path to the coordinator profile or NULL to consult the default profile path
 * @param profile Name of the Disnix profile that identifies the deployment (typically: default)
 * @return The provided manifest, previous manifest, or NULL in case of an error
 */
OldManifest *open_provided_or_previous_old_manifest_file(const gchar *manifest_file, const gchar *coordinator_profile_path, gchar *profile);

#endif
