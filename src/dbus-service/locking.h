/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2019  Sander van der Burg
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

#ifndef __DISNIX_LOCKING_H
#define __DISNIX_LOCKING_H

#include <unistd.h>
#include <glib.h>
#include <procreact_future.h>
#include <profilemanifest.h>

/**
 * Asynchronously executes the acquire_locks() operation in a child process.
 */
pid_t acquire_locks_async(int log_fd, gchar *tmpdir, ProfileManifest *profile_manifest, gchar *profile);

/**
 * Asynchronously executes the release_locks() operation in a child process.
 */
pid_t release_locks_async(int log_fd, gchar *tmpdir, ProfileManifest *profile_manifest, gchar *profile);

/**
 * Queries all the properties of the entries stored in the profile manifest.
 *
 * @param localstatedir Directory in which a user's Nix profiles are stored
 * @param profile Name of the Nix profile
 * @return A future that can be used to retrieve a NULL-terminated string array
 *   of derivation names
 */
ProcReact_Future query_installed_services(gchar *localstatedir, gchar *profile);

#endif
