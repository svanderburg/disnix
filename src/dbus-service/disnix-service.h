/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2021  Sander van der Burg
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

#ifndef __DISNIX_SERVICE_H
#define __DISNIX_SERVICE_H
#include <procreact_types.h>

/**
 * Starts the Disnix D-Bus service in the foreground
 *
 * @param session_bus Indicates whether the daemon should be registered on the session bus or system bus
 * @param log_path Directory in which log files are stored
 */
int start_disnix_service_foreground(ProcReact_bool session_bus, char *log_path);

/**
 * Daemonizes first and starts the Disnix D-Bus service in the background
 *
 * @param session_bus Indicates whether the daemon should be registered on the session bus or system bus
 * @param log_path Directory in which log files are stored
 * @param pid_file Path to the PID file that stores the PID of the daemon process
 * @param log_file Path to the log file storing general daemon messages
 */
int start_disnix_service_daemon(ProcReact_bool session_bus, char *log_path, char *pid_file, char *log_file);

#endif
