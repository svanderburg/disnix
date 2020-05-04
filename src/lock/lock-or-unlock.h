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

#ifndef __DISNIX_LOCK_OR_UNLOCK_H
#define __DISNIX_LOCK_OR_UNLOCK_H
#include <glib.h>

/**
 * Locks or unlocks the target machines in a manifest
 *
 * @param do_lock TRUE to lock the machines, FALSE to unlock them
 * @param manifest_file Path to the manifest file
 * @param coordinator_profile_path Path where the current deployment state is stored for future reference
 * @param profile Identifier of the distributed profile
 * @return 0 if the unlocking phase succeeds, else a non-zero exit status
 */
int lock_or_unlock(const int do_lock, const gchar *manifest_file, const gchar *coordinator_profile_path, gchar *profile);

#endif
