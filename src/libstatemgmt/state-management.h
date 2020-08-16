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

#ifndef __DISNIX_STATE_MANAGEMENT_H
#define __DISNIX_STATE_MANAGEMENT_H

#include <glib.h>
#include <unistd.h>
#include <procreact_future.h>

pid_t statemgmt_activate(gchar *type, gchar *component, gchar *container, char **arguments, int stdout_fd, int stderr_fd);

pid_t statemgmt_deactivate(gchar *type, gchar *component, gchar *container, char **arguments, int stdout_fd, int stderr_fd);

pid_t statemgmt_snapshot(gchar *type, gchar *component, gchar *container, char **arguments, int stdout_fd, int stderr_fd);

pid_t statemgmt_restore(gchar *type, gchar *component, gchar *container, char **arguments, int stdout_fd, int stderr_fd);

pid_t statemgmt_collect_garbage(gchar *type, gchar *component, gchar *container, char **arguments, int stdout_fd, int stderr_fd);

pid_t statemgmt_lock(gchar *type, gchar *container, gchar *component, int stdout_fd, int stderr_fd);

pid_t statemgmt_unlock(gchar *type, gchar *container, gchar *component, int stdout_fd, int stderr_fd);

pid_t statemgmt_shell(gchar *type, gchar *component, gchar *container, char **arguments, gchar *command);

gchar *statemgmt_capture_config(gchar *tmpdir, int stderr_fd, pid_t *pid, int *temp_fd);

#endif
