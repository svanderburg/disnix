/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2015  Sander van der Burg
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

#ifndef __DISNIX_CLIENT_OPERATION_H
#define __DISNIX_CLIENT_OPERATION_H
#include <glib.h>
#include <dbus/dbus-glib.h>
#include "operation.h"

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
 * @return 0 if the operation succeeds, else a non-zero exit value
 */
int run_disnix_client(Operation operation, gchar **derivation, gboolean session_bus, char *profile, gboolean delete_old, gchar **arguments, char *type);

#endif
