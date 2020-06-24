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

#ifndef __DISNIX_REMOTE_PACKAGE_MANAGEMENT_H
#define __DISNIX_REMOTE_PACKAGE_MANAGEMENT_H

#include <unistd.h>
#include <glib.h>
#include <procreact_future.h>

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
 * @return Future struct of the client interface process performing the operation
 */
ProcReact_Future exec_query_installed(gchar *interface, gchar *target, gchar *profile);

/**
 * Invokes the realise operation through a Disnix client interface
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param derivation Derivation to build
 * @return Future struct of the client interface process performing the operation
 */
ProcReact_Future exec_realise(gchar *interface, gchar *target, gchar *derivation);

/**
 * Queries the requisites of a given derivation
 *
 * @param interface Path to the interface executable
 * @param target Target Address of the remote interface
 * @param derivation Array of derivations to query the requisities from
 * @param derivation_length Length of the derivations array
 * @return Future struct of the client interface process performing the operation
 */
ProcReact_Future exec_query_requisites(gchar *interface, gchar *target, gchar **derivation, unsigned int derivation_length);

char **exec_query_requisites_sync(gchar *interface, gchar *target, gchar **derivation, unsigned int derivation_length);

ProcReact_Future exec_print_invalid(gchar *interface, gchar *target, gchar **paths, unsigned int paths_length);

char **exec_print_invalid_sync(gchar *interface, gchar *target, gchar **paths, unsigned int paths_length);

pid_t exec_import_local_closure(gchar *interface, gchar *target, char *closure);

ProcReact_bool exec_import_local_closure_sync(gchar *interface, gchar *target, char *closure);

ProcReact_Future exec_export_remote_closure(gchar *interface, gchar *target, char **paths, unsigned int paths_length);

char *exec_export_remote_closure_sync(gchar *interface, gchar *target, char **paths, unsigned int paths_length);

#endif
