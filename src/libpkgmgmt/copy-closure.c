/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2022  Sander van der Burg
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

#include "copy-closure.h"
#include <procreact_types.h>
#include "package-management.h"
#include "remote-package-management.h"

ProcReact_bool copy_closure_to_sync(gchar *interface, gchar *target, gchar *tmpdir, gchar **paths, int stderr_fd)
{
    char **requisites = pkgmgmt_query_requisites_sync(paths, g_strv_length(paths), stderr_fd);

    if(requisites == NULL)
        return FALSE;
    else
    {
        ProcReact_bool exit_status = TRUE;
        unsigned int requisites_length = g_strv_length(requisites);

        if(requisites_length > 0)
        {
            char **invalid_paths = pkgmgmt_remote_print_invalid_sync(interface, target, requisites, requisites_length);

            if(invalid_paths == NULL)
                exit_status = FALSE;
            else
            {
                unsigned int invalid_paths_length = g_strv_length(invalid_paths);

                if(invalid_paths_length > 0)
                {
                    char *tempfile = pkgmgmt_export_closure_sync(tmpdir, invalid_paths, invalid_paths_length, stderr_fd);

                    if(tempfile == NULL)
                        exit_status = FALSE;
                    else
                    {
                        exit_status = pkgmgmt_import_local_closure_sync(interface, target, tempfile);
                        unlink(tempfile);
                        g_free(tempfile);
                    }
                }

                procreact_free_string_array(invalid_paths);
            }
        }

        procreact_free_string_array(requisites);

        return exit_status;
    }
}

pid_t copy_closure_to(gchar *interface, gchar *target, gchar *tmpdir, gchar **paths, int stderr_fd)
{
    pid_t pid = fork();

    if(pid == 0)
        _exit(!copy_closure_to_sync(interface, target, tmpdir, paths, stderr_fd));

    return pid;
}

ProcReact_bool copy_closure_from_sync(gchar *interface, gchar *target, gchar **paths, int stdout_fd, int stderr_fd)
{
    char **requisites = pkgmgmt_remote_query_requisites_sync(interface, target, paths, g_strv_length(paths));

    if(requisites == NULL)
        return FALSE;
    else
    {
        ProcReact_bool exit_status = TRUE;
        unsigned int requisites_length = g_strv_length(requisites);

        if(requisites_length > 0)
        {
            char **invalid_paths = pkgmgmt_print_invalid_packages_sync(requisites, requisites_length, stderr_fd);

            if(invalid_paths == NULL)
                exit_status = FALSE;
            else
            {
                unsigned int invalid_paths_length = g_strv_length(invalid_paths);

                if(invalid_paths_length > 0)
                {
                    char *tempfile = pkgmgmt_export_remote_closure_sync(interface, target, invalid_paths, invalid_paths_length);

                    if(tempfile == NULL)
                        exit_status = FALSE;
                    else
                    {
                        exit_status = pkgmgmt_import_closure_sync(tempfile, stdout_fd, stderr_fd);
                        unlink(tempfile);
                        free(tempfile);
                    }
                }

                procreact_free_string_array(invalid_paths);
            }
        }

        procreact_free_string_array(requisites);

        return exit_status;
    }
}

pid_t copy_closure_from(gchar *interface, gchar *target, gchar **paths, int stdout_fd, int stderr_fd)
{
    pid_t pid = fork();

    if(pid == 0)
        _exit(!copy_closure_from_sync(interface, target, paths, stdout_fd, stderr_fd));

    return pid;
}
