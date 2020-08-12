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

#ifndef __DISNIX_COPYSNAPSHOTS_H
#define __DISNIX_COPYSNAPSHOTS_H
#include <glib.h>
#include <sys/types.h>
#include <procreact_util.h>

ProcReact_bool copy_snapshots_to_sync(gchar *interface, gchar *target, gchar *container, gchar *component, ProcReact_bool all, int stderr_fd);

pid_t copy_snapshots_to(gchar *interface, gchar *target, gchar *container, gchar *component, ProcReact_bool all, int stderr_fd);

ProcReact_bool copy_snapshots_from_sync(gchar *interface, gchar *target, gchar *container, gchar *component, ProcReact_bool all, int stdout_fd, int stderr_fd);

pid_t copy_snapshots_from(gchar *interface, gchar *target, gchar *container, gchar *component, ProcReact_bool all, int stdout_fd, int stderr_fd);

#endif
