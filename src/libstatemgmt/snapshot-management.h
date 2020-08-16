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

#ifndef __DISNIX_SNAPSHOT_MANAGEMENT_H
#define __DISNIX_SNAPSHOT_MANAGEMENT_H

#include <glib.h>
#include <unistd.h>
#include <procreact_future.h>

ProcReact_Future statemgmt_query_all_snapshots(gchar *container, gchar *component, int stderr_fd);

char **statemgmt_query_all_snapshots_sync(gchar *container, gchar *component, int stderr_fd);

ProcReact_Future statemgmt_query_latest_snapshot(gchar *container, gchar *component, int stderr_fd);

char **statemgmt_query_latest_snapshot_sync(gchar *container, gchar *component, int stderr_fd);

ProcReact_Future statemgmt_print_missing_snapshots(gchar **component, const unsigned int component_length, int stderr_fd);

char **statemgmt_print_missing_snapshots_sync(gchar **component, const unsigned int component_length, int stderr_fd);

char **statemgmt_resolve_snapshots_sync(gchar **snapshots, const unsigned int snapshots_length, int stderr_fd);

pid_t statemgmt_clean_snapshots(gint keep, gchar *container, gchar *component, int stdout_fd, int stderr_fd);

pid_t statemgmt_import_snapshots(gchar *container, gchar *component, gchar **resolved_snapshots, const unsigned int resolved_snapshots_length, int stdout_fd, int stderr_fd);

ProcReact_bool statemgmt_import_snapshots_sync(gchar *container, gchar *component, gchar **resolved_snapshots, const unsigned int resolved_snapshots_length, int stdout_fd, int stderr_fd);

ProcReact_Future statemgmt_resolve_snapshots(gchar **snapshots, const unsigned int snapshots_length, int stderr_fd);

#endif
