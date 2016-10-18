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

#ifndef __DISNIX_STATE_MANAGEMENT_H
#define __DISNIX_STATE_MANAGEMENT_H

#include <glib.h>
#include <unistd.h>

pid_t statemgmt_run_dysnomia_activity(gchar *type, gchar *activity, gchar *component, gchar *container, gchar **arguments, int stdout, int stderr);

pid_t statemgmt_query_all_snapshots(gchar *container, gchar *component, int pipefd[2], int stderr);

pid_t statemgmt_query_latest_snapshot(gchar *container, gchar *component, int pipefd[2], int stderr);

pid_t statemgmt_print_missing_snapshots(gchar **component, int pipefd[2], int stderr);

pid_t statemgmt_import_snapshots(gchar *container, gchar *component, gchar **snapshots, int stdout, int stderr);

pid_t statemgmt_resolve_snapshots(gchar **snapshots, int pipefd[2], int stderr);

pid_t statemgmt_clean_snapshots(gchar *keepStr, gchar *container, gchar *component, int stdout, int stderr);

gchar **statemgmt_capture_config(gchar *tmpdir, int stderr);

#endif
