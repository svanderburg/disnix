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

#ifndef __DISNIX_PROFILEMANIFEST_H
#define __DISNIX_PROFILEMANIFEST_H
#include <stdio.h>
#include <glib.h>

/**
 * @brief Exposes deployment properties that each individual target machine needs to know about itself
 */
typedef struct
{
    /** Hash table containing the properties of all services */
    GHashTable *services_table;

    /** Array of services to be activated on a target machine */
    GPtrArray *service_mapping_array;

    /** Array of snapshots to be tranferred to a target machine */
    GPtrArray *snapshot_mapping_array;
}
ProfileManifest;

/**
 * Composes a profile manifest struct instance from the manifest configuration
 * file stored in the given profile.
 *
 * @param result A string containing the XML document to parse
 * @return A profile manifest instance or NULL if an error has occured
 */
ProfileManifest *create_profile_manifest_from_string(char *result);

/**
 * Composes a profile manifest struct instance from the provided manifest
 * configuration file.
 *
 * @param profile_manifest_file Path to a profile manifest file
 * @return A profile manifest instance or NULL if an error has occured
 */
ProfileManifest *create_profile_manifest_from_file(const gchar *profile_manifest_file);

/**
 * Composes a profile manifest struct instance from the manifest configuration
 * file stored in the given profile.
 *
 * @param localstatedir Directory if which Nix's state files are stored
 * @param profile Name of the profile to take the manifest from
 * @return An array of pointers to profile manifest entries or NULL if an error has occured
 */
ProfileManifest *create_profile_manifest_from_current_deployment(gchar *localstatedir, gchar *profile);

/**
 * Deletes a profile manifest struct and its contents from heap memory.
 *
 * @param profile_manifest A profile manifest struct instance
 */
void delete_profile_manifest(ProfileManifest *profile_manifest);

/**
 * Prints a Nix expression representation of a profile manifest.
 *
 * @param file File descriptor to write to
 * @param profile_manifest A profile manifest struct instance
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_profile_manifest_nix(FILE *file, const ProfileManifest *profile_manifest, const int indent_level, void *userdata);

/**
 * Prints an XML representation of a profile manifest.
 *
 * @param file File descriptor to write to
 * @param profile_manifest A profile manifest struct instance
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_profile_manifest_xml(FILE *file, const ProfileManifest *profile_manifest, const int indent_level, const char *type_property_name, void *userdata);

/**
 * Prints the deployed services for the given profile manifest.
 *
 * @param profile_manifest A profile manifest struct instance
 */
void print_services_in_profile_manifest(const ProfileManifest *profile_manifest);

/**
 * Prints the deployed services grouped by container for the given profile
 * manifest.
 *
 * @param profile_manifest A profile manifest struct instance
 */
void print_services_per_container_in_profile_manifest(ProfileManifest *profile_manifest);

/**
 * Prints a raw textual (XML) representation of a profile manifest file.
 *
 * @param localstatedir Directory in which the Nix profiles are stored
 * @param profile Name of a Nix profile
 * @param fd File descriptor to print to
 */
void print_text_from_profile_manifest(gchar *localstatedir, gchar *profile, int fd);

#endif
