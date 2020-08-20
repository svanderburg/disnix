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

#ifndef __DISNIX_COPY_SNAPSHOTS_H
#define __DISNIX_COPY_SNAPSHOTS_H
#include <glib.h>
#include <sys/types.h>
#include <procreact_util.h>

/**
 * Copies generations of snapshots to a remote machine.
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param container Name of the container to deploy the snapshots to
 * @param component Name of the component to deploy the snapshots to
 * @param all TRUE to copy all snapshot generations, FALSE to only copy the latest
 * @param stderr_fd File descriptor to attach to the process' standard error
 * @return TRUE if the operation succeeded, else FALSE
 */
ProcReact_bool copy_snapshots_to_sync(gchar *interface, gchar *target, gchar *container, gchar *component, ProcReact_bool all, int stderr_fd);

/**
 * Asynchronously copies snapshots to a remote machine.
 *
 * @see copy_snapshots_to_sync
 */
pid_t copy_snapshots_to(gchar *interface, gchar *target, gchar *container, gchar *component, ProcReact_bool all, int stderr_fd);

/**
 * Copies generations of snapshots from a remote machine.
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param container Name of the container to deploy the snapshots to
 * @param component Name of the component to deploy the snapshots to
 * @param all TRUE to copy all snapshot generations, FALSE to only copy the latest
 * @param stdout_fd File descriptor to attach to the process' standard output
 * @param stderr_fd File descriptor to attach to the process' standard error
 * @return TRUE if the operation succeeded, else FALSE
 */
ProcReact_bool copy_snapshots_from_sync(gchar *interface, gchar *target, gchar *container, gchar *component, ProcReact_bool all, int stdout_fd, int stderr_fd);

/**
 * Asynchronously copies snapshots from a remote machine.
 *
 * @see copy_snapshots_from_sync
 */
pid_t copy_snapshots_from(gchar *interface, gchar *target, gchar *container, gchar *component, ProcReact_bool all, int stdout_fd, int stderr_fd);

#endif
