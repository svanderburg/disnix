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

#ifndef __DISNIX_LOGGING_H
#define __DISNIX_LOGGING_H
#include <glib.h>

/**
 * Configures the log directory.
 *
 * @param log_path Path to the directory in which the log files will be stored
 */
void set_logdir(char *log_path);

/**
 * Takes a string buffer, delimits the lines into arrays and appends it
 * to existing lines vector.
 *
 * @param lines An array of strings corresponding to a line of text
 * @param buf A string buffer that may contain linefeeds
 * @return An array of strings in which each element corresponds to a line
 */
gchar **update_lines_vector(gchar **lines, char *buf);

/**
 * Opens a writable file descriptor for the logfile of a given process id.
 * It also creates the log directory if it does not exists.
 *
 * @param pid PID of the job that needs to be logged
 * @return A file descriptor that can be written to
 */
int open_log_file(const gint pid);

/**
 * Prints the derivation paths to a given file descriptor
 *
 * @param fd Number of the file descriptor
 * @param derivation A NULL terminated array of derivation paths
 */
void print_paths(int fd, gchar **derivation);

#endif
