/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2018  Sander van der Burg
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

#include "locking.h"
#include <procreact_pid.h>
#include <procreact_future.h>
#include <profilelocking.h>

pid_t acquire_locks_async(int log_fd, gchar *tmpdir, GPtrArray *profile_manifest_array, gchar *profile)
{
    pid_t pid = fork();

    if(pid == 0)
        _exit(!acquire_locks(log_fd, tmpdir, profile_manifest_array, profile));

    return pid;
}

pid_t release_locks_async(int log_fd, gchar *tmpdir, GPtrArray *profile_manifest_array, gchar *profile)
{
    pid_t pid = fork();

    if(pid == 0)
        _exit(!release_locks(log_fd, tmpdir, profile_manifest_array, profile));

    return pid;
}

ProcReact_Future query_installed_services(GPtrArray *profile_manifest_array)
{
    ProcReact_Future future = procreact_initialize_future(procreact_create_string_array_type('\n'));

    if(future.pid == 0)
    {
        print_text_from_profile_manifest_array(profile_manifest_array, future.fd);
        _exit(0);
    }

    return future;
}
