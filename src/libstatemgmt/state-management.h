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

#ifndef __DISNIX_STATE_MANAGEMENT_H
#define __DISNIX_STATE_MANAGEMENT_H

#include <glib.h>
#include <unistd.h>
#include <procreact_future.h>

/**
 * Activates a component in a container.
 *
 * @param type Name of the module that manages a component life-cycle
 * @param component Name of the component
 * @param container Name of the container
 * @param arguments Array of name=value pairs representing the deployment parameters
 * @param stdout_fd File descriptor to attach to the process' standard output
 * @param stderr_fd File descriptor to attach to the process' standard error
 * @return Process id of the process that executes the task or -1 in case of a failure
 */
pid_t statemgmt_activate(gchar *type, gchar *component, gchar *container, char **arguments, int stdout_fd, int stderr_fd);

/**
 * Deactivates a component in a container.
 *
 * @param type Name of the module that manages a component life-cycle
 * @param component Name of the component
 * @param container Name of the container
 * @param arguments Array of name=value pairs representing the deployment parameters
 * @param stdout_fd File descriptor to attach to the process' standard output
 * @param stderr_fd File descriptor to attach to the process' standard error
 * @return Process id of the process that executes the task or -1 in case of a failure
 */
pid_t statemgmt_deactivate(gchar *type, gchar *component, gchar *container, char **arguments, int stdout_fd, int stderr_fd);

/**
 * Snapshots the state of a component in a container.
 *
 * @param type Name of the module that manages a component life-cycle
 * @param component Name of the component
 * @param container Name of the container
 * @param arguments Array of name=value pairs representing the deployment parameters
 * @param stdout_fd File descriptor to attach to the process' standard output
 * @param stderr_fd File descriptor to attach to the process' standard error
 * @return Process id of the process that executes the task or -1 in case of a failure
 */
pid_t statemgmt_snapshot(gchar *type, gchar *component, gchar *container, char **arguments, int stdout_fd, int stderr_fd);

/**
 * Restores the state of a component in a container.
 *
 * @param type Name of the module that manages a component life-cycle
 * @param component Name of the component
 * @param container Name of the container
 * @param arguments Array of name=value pairs representing the deployment parameters
 * @param stdout_fd File descriptor to attach to the process' standard output
 * @param stderr_fd File descriptor to attach to the process' standard error
 * @return Process id of the process that executes the task or -1 in case of a failure
 */
pid_t statemgmt_restore(gchar *type, gchar *component, gchar *container, char **arguments, int stdout_fd, int stderr_fd);

/**
 * Restores the state of a component in a container.
 *
 * @param type Name of the module that manages a component life-cycle
 * @param component Name of the component
 * @param container Name of the container
 * @param arguments Array of name=value pairs representing the deployment parameters
 * @param stdout_fd File descriptor to attach to the process' standard output
 * @param stderr_fd File descriptor to attach to the process' standard error
 * @return Process id of the process that executes the task or -1 in case of a failure
 */
pid_t statemgmt_collect_garbage(gchar *type, gchar *component, gchar *container, char **arguments, int stdout_fd, int stderr_fd);

/**
 * Requests the component to agree on locking access to the system.
 *
 * @param type Name of the module that manages a component life-cycle
 * @param component Name of the component
 * @param container Name of the container
 * @param stdout_fd File descriptor to attach to the process' standard output
 * @param stderr_fd File descriptor to attach to the process' standard error
 * @return Process id of the process that executes the task or -1 in case of a failure
 */
pid_t statemgmt_lock(gchar *type, gchar *container, gchar *component, int stdout_fd, int stderr_fd);

/**
 * Releases the lock of the component.
 *
 * @param type Name of the module that manages a component life-cycle
 * @param component Name of the component
 * @param container Name of the container
 * @param stdout_fd File descriptor to attach to the process' standard output
 * @param stderr_fd File descriptor to attach to the process' standard error
 * @return Process id of the process that executes the task or -1 in case of a failure
 */
pid_t statemgmt_unlock(gchar *type, gchar *container, gchar *component, int stdout_fd, int stderr_fd);

/**
 * Starts a diagnostic shell for a component in a container.
 *
 * @param type Name of the module that manages a component life-cycle
 * @param component Name of the component
 * @param container Name of the container
 * @param arguments Array of name=value pairs representing the deployment parameters
 * @param command Command to execute in the shell or NULL to run an interactive shell
 * @return Process id of the process that executes the task or -1 in case of a failure
 */
pid_t statemgmt_shell(gchar *type, gchar *component, gchar *container, char **arguments, gchar *command);

/**
 * Captures the general and container properties of the current system in a Nix expression.
 *
 * @param tmpdir Directory in which temp files are stored
 * @param stderr_fd File descriptor to attach to the process' standard error
 * @param pid Pointer to a PID in which the PID of the process that executes the task is stored
 * @param temp_fd Pointer to an integer in which the file descriptor to the temp file is stored
 * @return Path to a Nix expression file containing the configuration or NULL if the operation failed
 */
gchar *statemgmt_capture_config(gchar *tmpdir, int stderr_fd, pid_t *pid, int *temp_fd);

#endif
