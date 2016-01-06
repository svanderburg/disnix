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
#include <glib.h>

/**
 * Snapshots the state of services in the manifest.
 *
 * @param manifest_file Path to the manifest file which maps services to machines
 * @param max_concurrent_transfers Specifies the maximum amount of concurrent transfers
 * @param transfer_only Indicates the only snapshots must transferred but not taken
 * @param all Indicates whether all snapshot generations must be transferred
 * @param old_manifest Manifest file representing the old deployment configuration
 * @param coordinator_profile_path Path where the current deployment state is stored for future reference
 * @param profile Name of the distributed profile
 * @param no_upgrade Force Disnix to not look at the previous configuration 
 * @return 0 if everything succeeds, else a non-zero exit status
 */
int snapshot(const gchar *manifest_file, const unsigned int max_concurrent_transfers, const int transfer_only, const int all, const gchar *old_manifest, const gchar *coordinator_profile_path, gchar *profile, const gboolean no_upgrade);

#endif
