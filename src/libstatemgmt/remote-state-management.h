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

#ifndef __DISNIX_REMOTE_STATE_MANAGEMENT_H
#define __DISNIX_REMOTE_STATE_MANAGEMENT_H

#include <unistd.h>
#include <glib.h>
#include <procreact_future.h>

/**
 * Invokes the activate operation through a Disnix client interface
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param container Name of the container in which the component is deployed
 * @param type Type Type of the service
 * @param arguments String vector with activation arguments in the form key=value
 * @param arguments_size Size of the arguments string vector
 * @param service Service to activate
 * @return PID of the client interface process performing the operation, or -1 in case of a failure
 */
pid_t exec_activate(gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service);

/**
 * Invokes the deactivate operation through a Disnix client interface
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param container Name of the container in which the component is deployed
 * @param type Type Type of the service
 * @param arguments String vector with activation arguments in the form key=value
 * @param arguments_size Size of the arguments string vector
 * @param service Service to deactivate
 * @return PID of the client interface process performing the operation, or -1 in case of a failure
 */
pid_t exec_deactivate(gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service);

/**
 * Invokes the lock operation through a Disnix client interface
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param profile Identifier of the distributed profile
 * @return PID of the client interface process performing the operation, or -1 in case of a failure
 */
pid_t exec_lock(gchar *interface, gchar *target, gchar *profile);

/**
 * Invokes the unlock operation through a Disnix client interface
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param profile Identifier of the distributed profile
 * @return PID of the client interface process performing the operation, or -1 in case of a failure
 */
pid_t exec_unlock(gchar *interface, gchar *target, gchar *profile);

/**
 * Invokes the snapshot operation through a Disnix client interface
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param container Name of the container in which the component is deployed
 * @param type Type Type of the service
 * @param arguments String vector with activation arguments in the form key=value
 * @param arguments_size Size of the arguments string vector
 * @param service Service to activate
 * @return PID of the client interface process performing the operation, or -1 in case of a failure
 */
pid_t exec_snapshot(gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service);

/**
 * Invokes the restore operation through a Disnix client interface
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param container Name of the container in which the component is deployed
 * @param type Type Type of the service
 * @param arguments String vector with activation arguments in the form key=value
 * @param arguments_size Size of the arguments string vector
 * @param service Service to deactivate
 * @return PID of the client interface process performing the operation, or -1 in case of a failure
 */
pid_t exec_restore(gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service);

/**
 * Invokes the delete state operation through a Disnix client interface
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param container Name of the container in which the component is deployed
 * @param type Type Type of the service
 * @param arguments String vector with activation arguments in the form key=value
 * @param arguments_size Size of the arguments string vector
 * @param service Service to activate
 * @return PID of the client interface process performing the operation, or -1 in case of a failure
 */
pid_t exec_delete_state(gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service);

/**
 * Spawns a remote shell session with a Dysnomia shell.
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param container Name of the container in which the component is deployed
 * @param type Type Type of the service
 * @param arguments String vector with activation arguments in the form key=value
 * @param arguments_size Size of the arguments string vector
 * @param service Service to activate
 * @param command Shell command to execute or NULL to spawn an interactive session
 * @return PID of the client interface process performing the operation, or -1 in case of a failure
 */
pid_t exec_dysnomia_shell(gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service, gchar *command);

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
pid_t exec_clean_snapshots(gchar *interface, gchar *target, int keep, char *container, char *component);

/**
 * Captures the configuration from the Dysnomia container configuration files
 * and generates a Nix expression from it.
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @return Future struct of the client interface process performing the operation
 */
ProcReact_Future exec_capture_config(gchar *interface, gchar *target);

ProcReact_Future exec_query_all_snapshots(gchar *interface, gchar *target, gchar *container, gchar *component);

char **exec_query_all_snapshots_sync(gchar *interface, gchar *target, gchar *container, gchar *component);

ProcReact_Future exec_query_latest_snapshot(gchar *interface, gchar *target, gchar *container, gchar *component);

char **exec_query_latest_snapshot_sync(gchar *interface, gchar *target, gchar *container, gchar *component);

ProcReact_Future exec_print_missing_snapshots(gchar *interface, gchar *target, gchar **snapshots, unsigned int snapshots_length);

char **exec_print_missing_snapshots_sync(gchar *interface, gchar *target, gchar **snapshots, unsigned int snapshots_length);

ProcReact_Future exec_print_missing_snapshots(gchar *interface, gchar *target, gchar **snapshots, unsigned int snapshots_length);

char **exec_print_missing_snapshots_sync(gchar *interface, gchar *target, gchar **snapshots, unsigned int snapshots_length);

ProcReact_Future exec_resolve_snapshots(gchar *interface, gchar *target, gchar **snapshots, unsigned int snapshots_length);

char **exec_resolve_snapshots_sync(gchar *interface, gchar *target, gchar **snapshots, unsigned int snapshots_length);

pid_t exec_import_local_snapshots(gchar *interface, gchar *target, gchar *container, gchar *component, gchar **snapshots, unsigned int snapshots_length);

int exec_import_local_snapshots_sync(gchar *interface, gchar *target, gchar *container, gchar *component, gchar **snapshots, unsigned int snapshots_length);

pid_t exec_import_remote_snapshots(gchar *interface, gchar *target, gchar *container, gchar *component, gchar **snapshots, unsigned int snapshots_length);

int exec_import_remote_snapshots_sync(gchar *interface, gchar *target, gchar *container, gchar *component, gchar **snapshots, unsigned int snapshots_length);

ProcReact_Future exec_export_remote_snapshots(gchar *interface, gchar *target, gchar **snapshots, unsigned int snapshots_length);

char **exec_export_remote_snapshots_sync(gchar *interface, gchar *target, gchar **snapshots, unsigned int snapshots_length);

/**
 * Invokes the true command for testing purposes.
 */
pid_t exec_true(void);

#endif
