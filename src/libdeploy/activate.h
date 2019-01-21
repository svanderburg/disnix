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

#ifndef __DISNIX_ACTIVATE_H
#define __DISNIX_ACTIVATE_H
#include <glib.h>
#include <manifest.h>
#include "transition.h"

/**
 * Prints a human readable form of the transition status including instructions
 * explaining how to resolve issues.
 *
 * @param status A value from the TransitionStatus enumeration
 * @param old_manifest_file Path to the old manifest file
 * @param new_manifest_file Path to the new manifest file
 * @param coordinator_profile_path Path where the current deployment configuration must be stored
 * @param profile Name of the distributed profile
 */
void print_transition_status(TransitionStatus status, const gchar *old_manifest_file, const gchar *new_manifest_file, const gchar *coordinator_profile_path, const gchar *profile);

/**
 * Deactivates all obsolete services and activates all new services.
 *
 * @param manifest Manifest containing all deployment information of the new configuration
 * @param old_activation_mappings Array of activation mappings belonging to the previous configuration
 * @param Deployment option flags\
 * @return A value from the TransitionStatus enumeration
 */
TransitionStatus activate_system(Manifest *manifest, GPtrArray *old_activation_mappings, const unsigned int flags);

#endif
