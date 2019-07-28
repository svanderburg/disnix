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
#include "profilemanifest.h"

/**
 * @brief Contains all runtime properties of a target machine's deployment configuration
 */
typedef struct
{
    /** Path to the Nix store path of the profile containing the intra-dependency closures of all deployed services */
    gchar *profile;
    /** A profile manifest that captures all deployment properties of all services mapped to a machine */
    ProfileManifest *profile_manifest;
}
ProfileManifestTarget;

/**
 * Parses the manifest from the given profile and constructs a profile manifest
 * target instance from it.
 *
 * @param profile_path Path to a Nix profile containing all services deployed to a machine
 * @return A profile manifest target instance
 */
ProfileManifestTarget *parse_profile_manifest_target(gchar *profile_path);

/**
 * Deletes a profile manifest target from heap memory.
 *
 * @param profile_manifest_target A profile manifest target instance
 */
void delete_profile_manifest_target(ProfileManifestTarget *profile_manifest_target);

/**
 * Prints a Nix expression representation of a profile manifest target.
 *
 * @param file File descriptor to write to
 * @param profile_manifest_target A progile manifest target struct instance
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_profile_manifest_target_nix(FILE *file, const ProfileManifestTarget *profile_manifest_target, const int indent_level, void *userdata);

/**
 * Prints an XML representation of a profile manifest target.
 *
 * @param file File descriptor to write to
 * @param profile_manifest_target A progile manifest target struct instance
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_profile_manifest_target_xml(FILE *file, const ProfileManifestTarget *profile_manifest_target, const int indent_level, const char *type_property_name, void *userdata);

#endif
