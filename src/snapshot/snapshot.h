/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2016  Sander van der Burg
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

#ifndef __DISNIX_SNAPSHOT_H
#define __DISNIX_SNAPSHOT_H

#define FLAG_TRANSFER_ONLY 0x1
#define FLAG_DEPTH_FIRST 0x2
#define FLAG_ALL 0x4
#define FLAG_NO_UPGRADE 0x8

#include <glib.h>

/**
 * Snapshots the state of services in the manifest.
 *
 * @param manifest_file Path to the manifest file which maps services to machines
 * @param max_concurrent_transfers Specifies the maximum amount of concurrent transfers
 * @param keep Indicates how many snapshot generations should be kept
 * @param flags Option flags
 * @param old_manifest Manifest file representing the old deployment configuration
 * @param coordinator_profile_path Path where the current deployment state is stored for future reference
 * @param profile Name of the distributed profile
 * @param container Snapshot operations will be restricted to the given container, NULL indicates all containers
 * @param component Snapshot operations will be restricted to the given component, NULL indicates all components
 * @return 0 if everything succeeds, else a non-zero exit status
 */
int snapshot(const gchar *manifest_file, const unsigned int max_concurrent_transfers, const unsigned int flags, const int keep, const gchar *old_manifest, const gchar *coordinator_profile_path, gchar *profile, const gchar *container, const gchar *component);

#endif
