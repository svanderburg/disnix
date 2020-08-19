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

#ifndef __DISNIX_REMOTE_PACKAGE_MANAGEMENT_H
#define __DISNIX_REMOTE_PACKAGE_MANAGEMENT_H

#include <unistd.h>
#include <glib.h>
#include <procreact_future.h>

/**
 * Invokes the collect garbage operation through a Disnix client interface
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param delete_old Indicates whether old profile generations must be removed
 * @return PID of the client interface process performing the operation, or -1 in case of a failure
 */
pid_t pkgmgmt_remote_collect_garbage(gchar *interface, gchar *target, const ProcReact_bool delete_old);

/**
 * Invokes the set operation through a Disnix client interface
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param profile Identifier of the distributed profile
 * @param component Component which becomes the contents of the profile
 */
pid_t pkgmgmt_remote_set(gchar *interface, gchar *target, gchar *profile, gchar *component);

/**
 * Invokes the the query installed operation through a Disnix client interface
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param profile Identifier of the distributed profile
 * @return Future struct of the client interface process performing the operation
 */
ProcReact_Future pkgmgmt_remote_query_installed(gchar *interface, gchar *target, gchar *profile);

/**
 * Invokes the realise operation through a Disnix client interface
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param derivation Derivation to build
 * @return Future struct of the client interface process performing the operation
 */
ProcReact_Future pkgmgmt_remote_realise(gchar *interface, gchar *target, gchar *derivation);

/**
 * Queries the requisites of a given derivation through a Disnix client interface
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param paths Array of Nix store the paths to query the requisities from
 * @param paths_length Length of the paths array
 * @return Future struct of the client interface process performing the operation
 */
ProcReact_Future pkgmgmt_remote_query_requisites(gchar *interface, gchar *target, gchar **paths, const unsigned int paths_length);

/**
 * Synchronously queries the requisites for a collection of paths.
 *
 * @see pkgmgmt_remote_query_requisites
 * @return A string vector with all requisite paths
 */
char **pkgmgmt_remote_query_requisites_sync(gchar *interface, gchar *target, gchar **paths, const unsigned int paths_length);

/**
 * Invokes the the print invalid operation through a Disnix client interface.
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param paths Array of Nix store the paths to query the requisities from
 * @param paths_length Length of the paths array
 * @return A future that returns the invalid Nix store paths
 */
ProcReact_Future pkgmgmt_remote_print_invalid(gchar *interface, gchar *target, gchar **paths, const unsigned int paths_length);

/**
 * Synchronously prints invalid store paths.
 *
 * @see pkgmgmt_remote_print_invalid
 */
char **pkgmgmt_remote_print_invalid_sync(gchar *interface, gchar *target, gchar **paths, const unsigned int paths_length);

/**
 * Transfers a serialization of a closure of Nix store paths and imports it
 * on the remote machine.
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param closure Path to a closure serialization file
 * @return PID of the process that executes the task
 */
pid_t pkgmgmt_import_local_closure(gchar *interface, gchar *target, char *closure);

/**
 * Synchronously transfers and imports a closure file.
 *
 * @see pkgmgmt_import_local_closure
 */
ProcReact_bool pkgmgmt_import_local_closure_sync(gchar *interface, gchar *target, char *closure);

/**
 * Exports the closure of Nix stores paths on the remote machine and retrieves the result.
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param paths Array of Nix store the paths to query the requisities from
 * @param paths_length Length of the paths array
 * @return A future that returns the path to a temp file containing the serialization
 */
ProcReact_Future pkgmgmt_export_remote_closure(gchar *interface, gchar *target, char **paths, const unsigned int paths_length);

/**
 * Synchronously retrieves the closure of Nix store paths from the remote machine.
 *
 * @see pkgmgmt_export_remote_closure
 */
char *pkgmgmt_export_remote_closure_sync(gchar *interface, gchar *target, char **paths, const unsigned int paths_length);

#endif
