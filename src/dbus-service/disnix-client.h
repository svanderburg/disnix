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

#ifndef __DISNIX_CLIENT_H
#define __DISNIX_CLIENT_H
#include <glib.h>

/**
 * Enumeration of possible Disnix service operations
 */
typedef enum
{
    OP_NONE,
    OP_IMPORT,
    OP_EXPORT,
    OP_PRINT_INVALID,
    OP_REALISE,
    OP_SET,
    OP_QUERY_INSTALLED,
    OP_QUERY_REQUISITES,
    OP_COLLECT_GARBAGE,
    OP_ACTIVATE,
    OP_DEACTIVATE,
    OP_LOCK,
    OP_UNLOCK,
    OP_SNAPSHOT,
    OP_RESTORE,
    OP_QUERY_ALL_SNAPSHOTS,
    OP_QUERY_LATEST_SNAPSHOT,
    OP_PRINT_MISSING_SNAPSHOTS,
    OP_IMPORT_SNAPSHOTS,
    OP_RESOLVE_SNAPSHOTS,
    OP_CLEAN_SNAPSHOTS,
    OP_DELETE_STATE,
    OP_CAPTURE_CONFIG
}
Operation;

/**
 * Runs the Disnix client
 *
 * @param operation Operation number to execute
 * @param derivation List of paths to Nix store components
 * @param session_bus Indicates whether to use the session bus of D-Bus
 * @param profile Identifier of a distributed profile
 * @param delete_old Indicates whether to delete old profile generations
 * @param arguments List of activation arguments in key=value format
 * @param type Type of the service
 * @param container Name of the container in which snapshots must be deployed
 * @param component Name of a mutable component in a container
 * @param keep Amount of snapshot generations to keep
 * @return 0 if the operation succeeds, else a non-zero exit value
 */
int run_disnix_client(Operation operation, gchar **derivation, gboolean session_bus, char *profile, gboolean delete_old, gchar **arguments, char *type, char *container, char *component, int keep);

#endif
