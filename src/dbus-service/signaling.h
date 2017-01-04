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

#ifndef __DISNIX_SIGNALING_H
#define __DISNIX_SIGNALING_H

#include <unistd.h>
#include <glib.h>
#include <procreact_future.h>
#include "disnix-dbus.h"

/**
 * Spawns a thread that waits for process to complete and propagates a finish
 * signal when it yields TRUE or a failure signal when it yiels FALSE.
 *
 * @param pid PID of the running process
 * @param object A Disnix DBus interface object
 * @param jid Job ID of the running process
 * @param log_fd File descriptor of the job's logfile
 */
void signal_boolean_result(pid_t pid, OrgNixosDisnixDisnix *object, gint jid, int log_fd);

/**
 * Spawns a thread that waits for future to complete and propagates a success
 * signal with the result if it succeeds or a failure signal when it fails.
 *
 * @param future Future delivering a string vector
 * @param object A Disnix DBus interface object
 * @param jid Job ID of the running process
 * @param log_fd File descriptor of the job's logfile
 */
void signal_strv_result(ProcReact_Future future, OrgNixosDisnixDisnix *object, gint jid, int log_fd);

/**
 * Spawns a thread that waits for process that writes to a tempfile to complete
 * and propagates a success signal with the correspondng path if it succeeds or
 * a failure signal when it fails.
 *
 * @param pid PID of the running process
 * @param tempfilename String containing the path to the tempfile
 * @param temp_fd File descriptor writing to the tempfile
 * @param object A Disnix DBus interface object
 * @param jid Job ID of the running process
 * @param log_fd File descriptor of the job's logfile
 */
void signal_tempfile_result(pid_t pid, gchar *tempfilename, int temp_fd, OrgNixosDisnixDisnix *object, gint jid, int log_fd);

#endif
