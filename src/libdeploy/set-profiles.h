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

#ifndef __DISNIX_SET_PROFILES_H
#define __DISNIX_SET_PROFILES_H

#define SET_NO_COORDINATOR_PROFILE 0x1
#define SET_NO_TARGET_PROFILES 0x2

#include <glib.h>
#include <procreact_types.h>
#include <manifest.h>

/**
 * Updates the coordinator profile referring to the last deployed manifest and
 * target Disnix profiles containing the closures of all services deployed to a
 * target machine.
 *
 * @param manifest Manifest containing all deployment information
 * @param manifest_file Path to the manifest file
 * @param coordinator_profile_path Path where the current deployment configuration must be stored
 * @param profile Name of the distributed profile
 * @param flags Set option flags
 * @return TRUE if the profiles have been successfully set, else FALSE
 */
ProcReact_bool set_profiles(const Manifest *manifest, const gchar *manifest_file, const gchar *coordinator_profile_path, char *profile, const unsigned int flags);

#endif
