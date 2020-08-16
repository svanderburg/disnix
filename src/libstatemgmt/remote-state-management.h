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
pid_t statemgmt_remote_activate(gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service);

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
pid_t statemgmt_remote_deactivate(gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service);

/**
 * Invokes the lock operation through a Disnix client interface
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param profile Identifier of the distributed profile
 * @return PID of the client interface process performing the operation, or -1 in case of a failure
 */
pid_t statemgmt_remote_lock(gchar *interface, gchar *target, gchar *profile);

/**
 * Invokes the unlock operation through a Disnix client interface
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param profile Identifier of the distributed profile
 * @return PID of the client interface process performing the operation, or -1 in case of a failure
 */
pid_t statemgmt_remote_unlock(gchar *interface, gchar *target, gchar *profile);

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
pid_t statemgmt_remote_snapshot(gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service);

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
pid_t statemgmt_remote_restore(gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service);

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
pid_t statemgmt_remote_delete_state(gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service);

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
pid_t statemgmt_remote_shell(gchar *interface, gchar *target, gchar *container, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service, gchar *command);

/**
 * Captures the configuration from the Dysnomia container configuration files
 * and generates a Nix expression from it.
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @return Future struct of the client interface process performing the operation
 */
ProcReact_Future statemgmt_remote_capture_config(gchar *interface, gchar *target);

/**
 * Invokes a dummy command (the true command) for testing purposes.
 */
pid_t statemgmt_dummy_command(void);

#endif
