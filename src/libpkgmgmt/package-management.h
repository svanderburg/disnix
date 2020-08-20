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

#ifndef __DISNIX_PACKAGE_MANAGEMENT_H
#define __DISNIX_PACKAGE_MANAGEMENT_H

#include <glib.h>
#include <unistd.h>
#include <procreact_future.h>

/**
 * Imports a closure serialization into the Nix store.
 *
 * @param closure Path to a Nix store export dump
 * @param stdout_fd File descriptor to attach to the process' standard output
 * @param stderr_fd File descriptor to attach to the process' standard error
 * @return Process id of the process that executes the task or -1 in case of a failure
 */
pid_t pkgmgmt_import_closure(const char *closure, int stdout_fd, int stderr_fd);

/**
 * Synchronously imports a closure serialization into the Nix store.
 *
 * @see pkgmgmt_import_closure
 * @return TRUE if the operation succeeded, else FALSE
 */
ProcReact_bool pkgmgmt_import_closure_sync(const char *closure, int stdout_fd, int stderr_fd);

/**
 * Serializes a collection of Nix store paths into a file.
 *
 * @param tmpdir Directory in which temp files are stored
 * @param paths An array of Nix store paths
 * @param paths_length The length of the paths array
 * @param stderr_fd File descriptor to attach to the process' standard error
 * @param pid Pointer to a PID in which the PID of the process that executes the task is stored
 * @param temp_fd Pointer to an integer in which the file descriptor to the temp file is stored
 * @return Path to a temp file that contains the serialization, or NULL if the operation failed.
 */
gchar *pkgmgmt_export_closure(gchar *tmpdir, gchar **paths, const unsigned int paths_length, int stderr_fd, pid_t *pid, int *temp_fd);

/**
 * Synchronously serializes a collection of Nix store paths into a file.
 *
 * @see pkgmgmt_export_closure
 * @return Path to a temp file that contains the serialization, or NULL if the operation failed.
 */
gchar *pkgmgmt_export_closure_sync(gchar *tmpdir, gchar **paths, const unsigned int paths_length, int stderr_fd);

/**
 * Prints the Nix store paths of packages that are invalid.
 *
 * @param paths An array of Nix store paths
 * @param paths_length The length of the paths array
 * @param stderr_fd File descriptor to attach to the process' standard error
 * @return A future that returns a string array with missing Nix store paths
 */
ProcReact_Future pkgmgmt_print_invalid_packages(gchar **paths, const unsigned int paths_length, int stderr_fd);

/**
 * Synchronously prints the Nix store paths of packages that are invalid.
 *
 * @see pkgmgmt_print_invalid_packages
 */
char **pkgmgmt_print_invalid_packages_sync(gchar **paths, const unsigned int paths_length, int stderr_fd);

/**
 * Realizes (builds) the provided paths to store derivation files.
 *
 * @param derivation_paths An array with Nix store paths to derivation files
 * @param derivation_paths_length The length of the derivation paths array
 * @param stderr_fd File descriptor to attach to the process' standard error
 * @return A future that returns a string array with the Nix store paths of the build results
 */
ProcReact_Future pkgmgmt_realise(gchar **derivation_paths, const unsigned int derivation_paths_length, int stderr_fd);

/**
 * Updates a Nix profile reference to a given Nix store path.
 *
 * @param profile Name of the Nix profile
 * @param path A Nix store path
 * @param stdout_fd File descriptor to attach to the process' standard output
 * @param stderr_fd File descriptor to attach to the process' standard error
 * @return Process id of the process that executes the task or -1 in case of a failure
 */
pid_t pkgmgmt_set_profile(gchar *profile, gchar *path, int stdout_fd, int stderr_fd);

/**
 * Queries the requisites (dependencies) of a collection of Nix store paths.
 *
 * @param paths An array of Nix store paths
 * @param paths_length The length of the paths array
 * @param stderr_fd File descriptor to attach to the process' standard error
 * @return A future that returns a string array with Nix store paths containing all requisites
 */
ProcReact_Future pkgmgmt_query_requisites(gchar **paths, const unsigned int paths_length, int stderr_fd);

/**
 * Synchronously queries the requisites (dependencies) of a collection of Nix
 * store paths.
 *
 * @see pkgmgmt_query_requisites
 */
char **pkgmgmt_query_requisites_sync(gchar **paths, const unsigned int paths_length, int stderr_fd);

/**
 * Removes all packages that are no longer in use.
 *
 * @param delete_old Whether to also remove older profile generations
 * @param stdout_fd File descriptor to attach to the process' standard output
 * @param stderr_fd File descriptor to attach to the process' standard error
 * @return Process id of the process that executes the task or -1 in case of a failure
 */
pid_t pkgmgmt_collect_garbage(const ProcReact_bool delete_old, int stdout_fd, int stderr_fd);

/**
 * Normalizes an infrastructure model by augmenting missing properties with
 * default values.
 *
 * @param infrastructure_expr Path to an infrastructure model
 * @param default_target_property Default name of the target machine property that contains the connection string
 * @param default_client_interface Default executable that should establish a connection with a remote machine
 * @return A future that returns an XML serialization of the normalized infrastructure model
 */
ProcReact_Future pkgmgmt_normalize_infrastructure(gchar *infrastructure_expr, gchar *default_target_property, gchar *default_client_interface);

/**
 * Synchronously normalizes an infrastructure model.
 *
 * @see pkgmgmt_normalize_infrastructure
 */
char *pkgmgmt_normalize_infrastructure_sync(gchar *infrastructure_expr, gchar *default_target_property, gchar *default_client_interface);

/**
 * Updates the Nix profile on the coordinator machine that captures the
 * currently deployed configuration.
 *
 * @param coordinator_profile_path Path to a directory in which the profile symlinks are stored
 * @param manifest_file Manifest file of the currently deployed configuration
 * @param profile Name of the coordinator profile
 * @return TRUE if the operation succeeded, else FALSE
 */
ProcReact_bool pkgmgmt_set_coordinator_profile(const gchar *coordinator_profile_path, const gchar *manifest_file, const gchar *profile);

#endif
