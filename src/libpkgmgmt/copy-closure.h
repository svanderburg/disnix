/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2021  Sander van der Burg
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

#ifndef __DISNIX_COPY_CLOSURE_H
#define __DISNIX_COPY_CLOSURE_H
#include <glib.h>
#include <procreact_util.h>

/**
 * Copies a closure of a collection of a Nix store paths to a remote machine.
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param tmpdir Directory in which temp files are stored
 * @param paths An array of Nix store paths
 * @param stderr_fd File descriptor to attach to the process' standard error
 * @return TRUE if the operation succeeds, else FALSE
 */
ProcReact_bool copy_closure_to_sync(gchar *interface, gchar *target, gchar *tmpdir, gchar **paths, int stderr_fd);

/**
 * Asynchronously copies a closure to a machine in a sub process.
 *
 * @see copy_closure_to_sync
 */
pid_t copy_closure_to(gchar *interface, gchar *target, gchar *tmpdir, gchar **paths, int stderr_fd);

/**
 * Copies a closure of a collection of a Nix store paths from a remote machine.
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param paths An array of Nix store paths
 * @param stdout_fd File descriptor to attach to the process' standard output
 * @param stderr_fd File descriptor to attach to the process' standard error
 * @return TRUE if the operation succeeds, else FALSE
 */
ProcReact_bool copy_closure_from_sync(gchar *interface, gchar *target, gchar **paths, int stdout_fd, int stderr_fd);

/**
 * Asynchronously copies a closure from a machine in a sub process.
 *
 * @see copy_closure_from_sync
 */
pid_t copy_closure_from(gchar *interface, gchar *target, gchar **paths, int stdout_fd, int stderr_fd);

#endif
