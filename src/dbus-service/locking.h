/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2017  Sander van der Burg
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
 * Composes an array of profile manifest entries from the manifest configuration
 * file stored in the given profile.
 *
 * @param profile Name of the profile to take the manifest from
 * @return An array of pointers to profile manifest entries or NULL if an error has occured
 */
GPtrArray *create_profile_manifest_array(gchar *profile);

/**
 * Attempts to lock the disnix service instance by consulting the services in
 * the profile manifest and by locking the Disnix service itself.
 *
 * @param log_fd File descriptor of the log file that shows the output
 * @param profile_manifest_array An array of profile manifest entries
 * @param profile Name of the profile to take the manifest from
 * @return TRUE if and only if the locking succeeded
 */
int acquire_locks(int log_fd, GPtrArray *profile_manifest_array, gchar *profile);

/**
 * Asynchronously executes the acquire_locks() operation in a child process.
 */
pid_t acquire_locks_async(int log_fd, GPtrArray *profile_manifest_array, gchar *profile);

/**
 * Attempts to unlock the disnix service by consulting the services in the
 * profile manifest and by unlocking the Disnix service itself.
 *
 * @param log_fd File descriptor of the log file that shows the output
 * @param profile_manifest_array An array of profile manifest entries
 * @param profile Name of the profile to take the manifest from
 * @return TRUE if and only if the unlocking succeeded
 */
int release_locks(int log_fd, GPtrArray *profile_manifest_array, gchar *profile);

/**
 * Asynchronously executes the release_locks() operation in a child process.
 */
pid_t release_locks_async(int log_fd, GPtrArray *profile_manifest_array, gchar *profile);

/**
 * Queries all the properties of the entries stored in the profile manifest.
 *
 * @param profile_manifest_array An array of profile manifest entries
 * @return A future that can be used to retrieve a NULL-terminated string array
 *   of derivation names
 */
ProcReact_Future query_installed_services(GPtrArray *profile_manifest_array);

#endif
