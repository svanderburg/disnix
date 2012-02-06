/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2011  Sander van der Burg
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

#ifndef __DISNIX_PROFILES_H
#define __DISNIX_PROFILES_H
#include <glib.h>

/**
 * Sets the Disnix profiles on the target machine, so that the services are
 * no longer garbage to the Nix package manager.
 *
 * @param distribution_array Array with Nix profiles containing installed services for each machine
 * @param interface Path to the client interface executable
 * @param profile Name of the distributed profile
 * @return 0 if everything succeeds, else a non-zero exit status
 */
int set_target_profiles(const GArray *distribution_array, gchar *interface, gchar *profile);

/**
 * Sets the Disnix coordinator profile, so that the current configuration is 
 * known, which we can use for efficient future upgrades.
 *
 * @param coordinator_profile Path where the current deployment configuration must be stored
 * @param manifest_file Path to the manifest file representing the deployment state
 * @param profile Name of the distributed profile
 * @param username Username of the current user invoking the activation
 * @return 0 if everything succeeds, else a non-zero exit status
 */
int set_coordinator_profile(const gchar *coordinator_profile_path, const gchar *manifest_file, const gchar *profile, const gchar *username);

#endif
