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

#ifndef __DISNIX_REMOTE_SNAPSHOT_MANAGEMENT_H
#define __DISNIX_REMOTE_SNAPSHOT_MANAGEMENT_H

#include <unistd.h>
#include <glib.h>
#include <procreact_future.h>

/**
 * Invokes the Dysnomia snapshot garbage collect operation through a Disnix client interface
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param keep Number of snapshot generations to keep
 * @param container Name of the container to filter on, or NULL to consult all containers
 * @param component Name of the component to filter on, or NULL to consult all components
 * @return PID of the client interface process performing the operation, or -1 in case of a failure
 */
pid_t statemgmt_remote_clean_snapshots(gchar *interface, gchar *target, int keep, char *container, char *component);

ProcReact_Future statemgmt_remote_query_all_snapshots(gchar *interface, gchar *target, gchar *container, gchar *component);

char **statemgmt_remote_query_all_snapshots_sync(gchar *interface, gchar *target, gchar *container, gchar *component);

ProcReact_Future statemgmt_remote_query_latest_snapshot(gchar *interface, gchar *target, gchar *container, gchar *component);

char **statemgmt_remote_query_latest_snapshot_sync(gchar *interface, gchar *target, gchar *container, gchar *component);

ProcReact_Future statemgmt_remote_print_missing_snapshots(gchar *interface, gchar *target, gchar **snapshots, const unsigned int snapshots_length);

char **statemgmt_remote_print_missing_snapshots_sync(gchar *interface, gchar *target, gchar **snapshots, const unsigned int snapshots_length);

ProcReact_Future statemgmt_remote_resolve_snapshots(gchar *interface, gchar *target, gchar **snapshots, const unsigned int snapshots_length);

char **statemgmt_remote_resolve_snapshots_sync(gchar *interface, gchar *target, gchar **snapshots, const unsigned int snapshots_length);

pid_t statemgmt_import_local_snapshots(gchar *interface, gchar *target, gchar *container, gchar *component, gchar **resolved_snapshots, const unsigned int resolved_snapshots_length);

ProcReact_bool statemgmt_import_local_snapshots_sync(gchar *interface, gchar *target, gchar *container, gchar *component, gchar **resolved_snapshots, const unsigned int resolved_snapshots_length);

pid_t statemgmt_import_remote_snapshots(gchar *interface, gchar *target, gchar *container, gchar *component, gchar **resolved_snapshots, const unsigned int resolved_snapshots_length);

ProcReact_bool statemgmt_import_remote_snapshots_sync(gchar *interface, gchar *target, gchar *container, gchar *component, gchar **resolved_snapshots, const unsigned int resolved_snapshots_length);

ProcReact_Future statemgmt_export_remote_snapshots(gchar *interface, gchar *target, gchar **resolved_snapshots, const unsigned int resolved_snapshots_length);

char **statemgmt_export_remote_snapshots_sync(gchar *interface, gchar *target, gchar **resolved_snapshots, const unsigned int resolved_snapshots_length);

#endif
