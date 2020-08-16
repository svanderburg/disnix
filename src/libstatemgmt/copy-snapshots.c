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

#include "copy-snapshots.h"
#define _XOPEN_SOURCE 500
#define __USE_XOPEN_EXTENDED 1
#include <ftw.h>
#include <libgen.h>
#include <unistd.h>
#include <stdio.h>
#include <procreact_types.h>
#include "snapshot-management.h"
#include "remote-snapshot-management.h"

static ProcReact_bool order_snapshots_remotely(gchar *interface, gchar *target, gchar *container, gchar *component, char **snapshot_array, const unsigned int snapshot_array_length)
{
    char **resolved_snapshots = statemgmt_remote_resolve_snapshots_sync(interface, target, snapshot_array, snapshot_array_length);
    ProcReact_bool exit_status = statemgmt_import_remote_snapshots_sync(interface, target, container, component, resolved_snapshots, g_strv_length(resolved_snapshots));
    procreact_free_string_array(resolved_snapshots);
    return exit_status;
}

static ProcReact_bool send_missing_snapshots(gchar *interface, gchar *target, gchar *container, gchar *component, char **missing_snapshots, const unsigned int missing_snapshots_length, int stderr_fd)
{
    char **resolved_snapshots = statemgmt_resolve_snapshots_sync(missing_snapshots, missing_snapshots_length, stderr_fd);

    if(resolved_snapshots == NULL)
        return FALSE;
    else
    {
        const unsigned int resolved_snapshots_length = g_strv_length(resolved_snapshots);
        ProcReact_bool exit_status;

        if(resolved_snapshots_length == 0)
            exit_status = FALSE;
        else
            exit_status = statemgmt_import_local_snapshots_sync(interface, target, container, component, resolved_snapshots, resolved_snapshots_length);

        procreact_free_string_array(resolved_snapshots);

        return exit_status;
    }
}

static char **query_local_snapshots(gchar *container, gchar *component, ProcReact_bool all, int stderr_fd)
{
    if(all)
        return statemgmt_query_all_snapshots_sync(container, component, stderr_fd);
    else
        return statemgmt_query_latest_snapshot_sync(container, component, stderr_fd);
}

ProcReact_bool copy_snapshots_to_sync(gchar *interface, gchar *target, gchar *container, gchar *component, ProcReact_bool all, int stderr_fd)
{
    char **snapshots = query_local_snapshots(container, component, all, stderr_fd);

    if(snapshots == NULL)
        return FALSE;
    else
    {
        ProcReact_bool exit_status = TRUE;
        unsigned int i;

        for(i = 0; i < g_strv_length(snapshots); i++) // We need to traverse the snapshots in the right order, one by one, to ensure that the generations are imported in the right order
        {
            char *snapshot = snapshots[i];
            char *snapshot_array[] = { snapshot, NULL };
            unsigned int snapshot_array_length = 1; // Length of the above array

            char **missing_snapshots = statemgmt_remote_print_missing_snapshots_sync(interface, target, snapshot_array, snapshot_array_length);

            if(missing_snapshots == NULL)
                exit_status = FALSE;
            else
            {
                unsigned int missing_snapshots_length = g_strv_length(missing_snapshots);

                if(missing_snapshots_length == 0) // If no snapshots need to be transferred, we still have to order them
                     exit_status = order_snapshots_remotely(interface, target, container, component, snapshot_array, snapshot_array_length);
                else
                     exit_status = send_missing_snapshots(interface, target, container, component, missing_snapshots, missing_snapshots_length, stderr_fd);

                procreact_free_string_array(missing_snapshots);
            }
        }

        procreact_free_string_array(snapshots);

        return exit_status;
    }
}

pid_t copy_snapshots_to(gchar *interface, gchar *target, gchar *container, gchar *component, ProcReact_bool all, int stderr_fd)
{
    pid_t pid = fork();

    if(pid == 0)
        _exit(!copy_snapshots_to_sync(interface, target, container, component, all, stderr_fd));

    return pid;
}

static int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    int rv = remove(fpath);

    if(rv == -1)
        fprintf(stderr, "Cannot remove: %s\n", fpath);

    return rv;
}

static ProcReact_bool remove_directory_and_contents(char *path)
{
    return (nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS) == 0);
}

static ProcReact_bool order_local_snapshot(gchar *container, gchar *component, char *snapshot, int stdout_fd, int stderr_fd)
{
    char *snapshot_array[] = { snapshot, NULL };
    char **resolved_snapshots = statemgmt_resolve_snapshots_sync(snapshot_array, g_strv_length(snapshot_array), stderr_fd);

    if(resolved_snapshots == NULL)
        return FALSE;
    else
    {
        ProcReact_bool exit_status = statemgmt_import_snapshots_sync(container, component, resolved_snapshots, g_strv_length(resolved_snapshots), stdout_fd, stderr_fd);
        procreact_free_string_array(resolved_snapshots);
        return exit_status;
    }
}

static ProcReact_bool retrieve_missing_snapshots(gchar *interface, gchar *target, gchar *container, gchar *component, char **missing_snapshots, int stdout_fd, int stderr_fd)
{
    char **resolved_snapshots = statemgmt_remote_resolve_snapshots_sync(interface, target, missing_snapshots, g_strv_length(missing_snapshots));

    if(resolved_snapshots == NULL)
        return FALSE;
    else
    {
        ProcReact_bool exit_status = TRUE;
        unsigned int resolved_snapshots_length = g_strv_length(resolved_snapshots);

        if(resolved_snapshots_length == 0)
            exit_status = FALSE;
        else
        {
            char **tmpdirs = statemgmt_export_remote_snapshots_sync(interface, target, resolved_snapshots, resolved_snapshots_length);

            if(tmpdirs == NULL)
                exit_status = FALSE;
            else
            {
                if(g_strv_length(tmpdirs) == 0)
                    exit_status = FALSE;
                else
                {
                    char *tmpdir = tmpdirs[0];
                    gchar *tmp_snapshot = g_strconcat(tmpdir, "/", basename(resolved_snapshots[0]), NULL);
                    char *tmp_snapshots[] = { tmp_snapshot, NULL };
                    const unsigned int tmp_snapshots_length = 1; // Length of the array above

                    exit_status = statemgmt_import_snapshots_sync(container, component, tmp_snapshots, tmp_snapshots_length, stdout_fd, stderr_fd);

                    if(exit_status == 0)
                        rmdir(tmpdir);
                    else
                        remove_directory_and_contents(tmpdir);

                    g_free(tmp_snapshot);
                }

                g_strfreev(tmpdirs);
            }
        }

        procreact_free_string_array(resolved_snapshots);

        return exit_status;
    }
}

static char **query_remote_snapshots(gchar *interface, gchar *target, gchar *container, gchar *component, ProcReact_bool all)
{
    if(all)
        return statemgmt_remote_query_all_snapshots_sync(interface, target, container, component);
    else
        return statemgmt_remote_query_latest_snapshot_sync(interface, target, container, component);
}

ProcReact_bool copy_snapshots_from_sync(gchar *interface, gchar *target, gchar *container, gchar *component, ProcReact_bool all, int stdout_fd, int stderr_fd)
{
    char **snapshots = query_remote_snapshots(interface, target, container, component, all);

    if(snapshots == NULL)
        return FALSE;
    else
    {
        ProcReact_bool exit_status = TRUE;
        unsigned int i;
        unsigned int snapshots_length = g_strv_length(snapshots);

        for(i = 0; i < snapshots_length; i++) // We need to traverse the snapshots in the right order, one by one, to ensure that the generations are imported in the right order
        {
            char *snapshot = snapshots[i];

            char **missing_snapshots = statemgmt_print_missing_snapshots_sync(snapshots, snapshots_length, stderr_fd);

            if(missing_snapshots == NULL)
                exit_status = FALSE;
            else
            {
                if(g_strv_length(missing_snapshots) == 0) // If no snapshots need to be transferred, we still have to order them
                    exit_status = order_local_snapshot(container, component, snapshot, stdout_fd, stderr_fd);
                else
                    exit_status = retrieve_missing_snapshots(interface, target, container, component, missing_snapshots, stdout_fd, stderr_fd);

                procreact_free_string_array(missing_snapshots);
            }
        }

        procreact_free_string_array(snapshots);

        return exit_status;
    }
}

pid_t copy_snapshots_from(gchar *interface, gchar *target, gchar *container, gchar *component, ProcReact_bool all, int stdout_fd, int stderr_fd)
{
    pid_t pid = fork();

    if(pid == 0)
        _exit(!copy_snapshots_from_sync(interface, target, container, component, all, stdout_fd, stderr_fd));

    return pid;
}
