/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2021  Sander van der Burg
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

#ifndef __DISNIX_PROFILEMANIFESTTARGETTABLE_H
#define __DISNIX_PROFILEMANIFESTTARGETTABLE_H

#include <glib.h>
#include "profilemanifesttarget.h"

/**
 * Deletes the given profile manifest target table and its properties from
 * heap memory.
 *
 * @param profile_manifest_target_table Hash table of profile manifest targets
 */
void delete_profile_manifest_target_table(GHashTable *profile_manifest_target_table);

/**
 * Checks whether all profile manifest targets in the hash table are valid.
 *
 * @param profile_manifest_target_table Hash table of profile manifest targets
 * @return TRUE if the profile manifest targets are valid, else FALSE
 */
int check_profile_manifest_target_table(GHashTable *profile_manifest_target_table);

/**
 * Prints for each target the deployed services.
 *
 * @param profile_manifest_target_table Hash table of profile manifest targets
 */
void print_services_in_profile_manifest_target_table(GHashTable *profile_manifest_target_table);

/**
 * Prints for each target the deployed services per container.
 *
 * @param profile_manifest_target_table Hash table of profile manifest targets
 */
void print_services_per_container_in_profile_manifest_target_table(GHashTable *profile_manifest_target_table);

/**
 * Prints a Nix expression containing profile mappings and service mappings
 * derived from the profile manifests on all machines. This expression can be
 * used to reconstruct the deployment manifest on the coordinator.
 *
 * @param profile_manifest_target_table Hash table of profile manifest targets
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_profile_manifest_target_table_nix(GHashTable *profile_manifest_target_table, void *userdata);

/**
 * Prints a XML file containing profile mappings and service mappings derived
 * from the profile manifests on all machines.
 *
 * @param profile_manifest_target_table Hash table of profile manifest targets
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_profile_manifest_target_table_xml(GHashTable *profile_manifest_target_table, void *userdata);

#endif
