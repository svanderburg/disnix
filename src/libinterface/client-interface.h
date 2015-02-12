/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2010  Sander van der Burg
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

#ifndef __DISNIX_CLIENT_INTERFACE_H
#define __DISNIX_CLIENT_INTERFACE_H

#include <unistd.h>
#include <glib.h>

/**
 * Waits until the given PID is finished and then returns the exit status
 *
 * @param pid PID of a process
 * @return Non-zero value in case of a failure
 */
int wait_to_finish(const pid_t pid);

/**
 * Invokes the activate operation through a Disnix client interface
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param type Type Type of the service
 * @param arguments String vector with activation arguments in the form key=value
 * @param arguments_size Size of the arguments string vector
 * @param service Service to activate
 * @return PID of the client interface process performing the operation, or -1 in case of a failure
 */
pid_t exec_activate(gchar *interface, gchar *target, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service);

/**
 * Invokes the deactivate operation through a Disnix client interface
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param type Type Type of the service
 * @param arguments String vector with activation arguments in the form key=value
 * @param arguments_size Size of the arguments string vector
 * @param service Service to deactivate
 * @return PID of the client interface process performing the operation, or -1 in case of a failure
 */
pid_t exec_deactivate(gchar *interface, gchar *target, gchar *type, gchar **arguments, const unsigned int arguments_size, gchar *service);

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
 * Invokes the collect garbage operation through a Disnix client interface
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param delete_old Indicates whether old profile generations must be removed
 * @return PID of the client interface process performing the operation, or -1 in case of a failure
 */
pid_t exec_collect_garbage(gchar *interface, gchar *target, const gboolean delete_old);

/**
 * Invokes the set operation through a Disnix client interface
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param profile Identifier of the distributed profile
 * @param component Component which becomes the contents of the profile
 */
pid_t exec_set(gchar *interface, gchar *target, gchar *profile, gchar *component);

/**
 * Invokes the the query installed operation through a Disnix client interface
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param profile Identifier of the distributed profile
 * @return PID of the client interface process performing the operation, or -1 in case of a failure
 */
pid_t exec_query_installed(gchar *interface, gchar *target, gchar *profile);

/**
 * Invokes the copy closure process to copy a closure from a machine
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param component Component to copy (including all intra-dependencies)
 * @return PID of the client interface process performing the operation, or -1 in case of a failure
 */ 
pid_t exec_copy_closure_from(gchar *interface, gchar *target, gchar *component);

/**
 * Invokes the copy closure process to copy a closure to a machine
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param component Component to copy (including all intra-dependencies)
 * @return PID of the client interface process performing the operation, or -1 in case of a failure
 */ 
pid_t exec_copy_closure_to(gchar *interface, gchar *target, gchar *component);

/**
 * Invokes the realise operation through a Disnix client interface
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param derivation Derivation to build
 * @param pipefd Pipe which can be used to capture the output of the process
 * @return PID of the client interface process performing the operation, or -1 in case of a failure
 */
pid_t exec_realise(gchar *interface, gchar *target, gchar *derivation, int pipefd[2]);

pid_t exec_true(void);

#endif
