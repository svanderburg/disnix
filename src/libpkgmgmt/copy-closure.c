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

#include "copy-closure.h"
#include <procreact_types.h>
#include "package-management.h"
#include "remote-package-management.h"

ProcReact_bool copy_closure_to_sync(gchar *interface, gchar *target, gchar *tmpdir, gchar **derivation, int stderr_fd)
{
    char **requisites = pkgmgmt_query_requisites_sync(derivation, stderr_fd);

    if(requisites == NULL)
        return FALSE;
    else
    {
        ProcReact_bool exit_status = TRUE;

        if(g_strv_length(requisites) > 0)
        {
            char **invalid_paths = pkgmgmt_remote_print_invalid_sync(interface, target, requisites, g_strv_length(requisites));

            if(invalid_paths == NULL)
                exit_status = FALSE;
            else
            {
                if(g_strv_length(invalid_paths) > 0)
                {
                    char *tempfile = pkgmgmt_export_closure_sync(tmpdir, invalid_paths, stderr_fd);

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

pid_t copy_closure_to(gchar *interface, gchar *target, gchar *tmpdir, gchar **derivation, int stderr_fd)
{
    pid_t pid = fork();

    if(pid == 0)
        _exit(!copy_closure_to_sync(interface, target, tmpdir, derivation, stderr_fd));

    return pid;
}

ProcReact_bool copy_closure_from_sync(gchar *interface, gchar *target, gchar **derivation, int stdout_fd, int stderr_fd)
{
    char **requisites = pkgmgmt_remote_query_requisites_sync(interface, target, derivation, g_strv_length(derivation));

    if(requisites == NULL)
        return FALSE;
    else
    {
        ProcReact_bool exit_status = TRUE;

        if(g_strv_length(requisites) > 0)
        {
            char **invalid_paths = pkgmgmt_print_invalid_packages_sync(requisites, stderr_fd);

            if(invalid_paths == NULL)
                exit_status = FALSE;
            else
            {
                if(g_strv_length(invalid_paths) > 0)
                {
                    char *tempfile = pkgmgmt_export_remote_closure_sync(interface, target, invalid_paths, g_strv_length(requisites));

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

pid_t copy_closure_from(gchar *interface, gchar *target, gchar **derivation, int stdout_fd, int stderr_fd)
{
    pid_t pid = fork();

    if(pid == 0)
        _exit(!copy_closure_from_sync(interface, target, derivation, stdout_fd, stderr_fd));

    return pid;
}
