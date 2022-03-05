/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2022  Sander van der Burg
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

#ifndef __DISNIX_SNAPSHOT_MANAGEMENT_H
#define __DISNIX_SNAPSHOT_MANAGEMENT_H

#include <glib.h>
#include <unistd.h>
#include <procreact_future.h>

/**
 * Queries all snapshot generations.
 *
 * @param container Name of the container to filter on, or NULL to consult all containers
 * @param component Name of the component to filter on, or NULL to consult all components
 * @param stderr_fd File descriptor to attach to the process' standard error
 * @return A future that returns snapshot names
 */
ProcReact_Future statemgmt_query_all_snapshots(gchar *container, gchar *component, int stderr_fd);

/**
 * Synchronously queries all snapshot generations.
 *
 * @see statemgmt_query_all_snapshots
 */
char **statemgmt_query_all_snapshots_sync(gchar *container, gchar *component, int stderr_fd);

/**
 * Queries the latest snapshot generation.
 *
 * @param container Name of the container to filter on, or NULL to consult all containers
 * @param component Name of the component to filter on, or NULL to consult all components
 * @param stderr_fd File descriptor to attach to the process' standard error
 * @return A future that returns snapshot names
 */
ProcReact_Future statemgmt_query_latest_snapshot(gchar *container, gchar *component, int stderr_fd);

/**
 * Synchronously queries the latest snapshot generation.
 *
 * @see statemgmt_query_latest_snapshot
 */
char **statemgmt_query_latest_snapshot_sync(gchar *container, gchar *component, int stderr_fd);

/**
 * Prints the names of all snapshots that are not in the snapshot store.
 *
 * @param snapshots An array of snapshot names
 * @param snapshots_length Length of the snapshots array
 * @param stderr_fd File descriptor to attach to the process' standard error
 * @return A future that returns snapshot names
 */
ProcReact_Future statemgmt_print_missing_snapshots(gchar **snapshots, const unsigned int snapshots_length, int stderr_fd);

/**
 * Synchronously prints the names of all snapshots that are not in the snapshot store.
 *
 * @see statemgmt_print_missing_snapshots
 */
char **statemgmt_print_missing_snapshots_sync(gchar **snapshots, const unsigned int snapshots_length, int stderr_fd);

/**
 * Resolves the names of the snapshots to absolute paths on the file system.
 *
 * @param snapshots An array of snapshot names
 * @param snapshots_length Length of the snapshots array
 * @param stderr_fd File descriptor to attach to the process' standard error
 * @return An array of resolved snapshot paths
 */
ProcReact_Future statemgmt_resolve_snapshots(gchar **snapshots, const unsigned int snapshots_length, int stderr_fd);

/**
 * Synchronously resolves the names of the snapshots to absolute paths on the file system.
 *
 * @see statemgmt_resolve_snapshots
 */
char **statemgmt_resolve_snapshots_sync(gchar **snapshots, const unsigned int snapshots_length, int stderr_fd);

/**
 * Cleans obsolete snapshot generations.
 *
 * @param keep Number of snapshot generations to keep
 * @param container Name of the container to filter on, or NULL to consult all containers
 * @param component Name of the component to filter on, or NULL to consult all components
 * @param stdout_fd File descriptor to attach to the process' standard output
 * @param stderr_fd File descriptor to attach to the process' standard error
 * @return Process id of the process that executes the task or -1 in case of a failure
 */
pid_t statemgmt_clean_snapshots(int keep, gchar *container, gchar *component, int stdout_fd, int stderr_fd);

/**
 * Imports all resolved snapshots into the local snapshot store.
 *
 * @param container Name of the container to filter on, or NULL to consult all containers
 * @param component Name of the component to filter on, or NULL to consult all components
 * @param resolved_snapshots Absolute paths to snapshots to be imported
 * @param resolved_snapshots_length Length of the resolved snapshots array
 * @param stdout_fd File descriptor to attach to the process' standard output
 * @param stderr_fd File descriptor to attach to the process' standard error
 * @return Process id of the process that executes the task or -1 in case of a failure
 */
pid_t statemgmt_import_snapshots(gchar *container, gchar *component, gchar **resolved_snapshots, const unsigned int resolved_snapshots_length, int stdout_fd, int stderr_fd);

/**
 * Synchronously imports all resolved snapshots into the local snapshot store.
 *
 * @see statemgmt_import_snapshots
 */
ProcReact_bool statemgmt_import_snapshots_sync(gchar *container, gchar *component, gchar **resolved_snapshots, const unsigned int resolved_snapshots_length, int stdout_fd, int stderr_fd);

#endif
