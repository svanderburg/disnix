/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2017  Sander van der Burg
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

#ifndef __DISNIX_LOGGING_H
#define __DISNIX_LOGGING_H
#include <glib.h>
#include "disnix-dbus.h"

/**
 * Configures the log directory.
 *
 * @param log_path Path to the directory in which the log files will be stored
 */
void set_logdir(char *log_path);

/**
 * Opens a writable file descriptor for the logfile of a given process id.
 * It also creates the log directory if it does not exists.
 *
 * @param object A Disnix DBus interface object
 * @param pid PID of the job that needs to be logged
 * @return A file descriptor that can be written to
 */
int open_log_file(OrgNixosDisnixDisnix *object, const gint pid);

/**
 * Prints the derivation paths to a given file descriptor
 *
 * @param fd Number of the file descriptor
 * @param derivation A NULL terminated array of derivation paths
 */
void print_paths(int fd, gchar **derivation);

#endif
