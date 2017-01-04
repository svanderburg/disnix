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

#include "logging.h"
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/** Path to the log directory */
char *logdir;

void set_logdir(char *log_path)
{
    logdir = log_path;
}

int open_log_file(OrgNixosDisnixDisnix *object, const gint pid)
{
    gchar pidStr[15];
    gchar *log_path;
    int log_fd;
    
    sprintf(pidStr, "%d", pid);
    
    mkdir(logdir, 0755);
    log_path = g_strconcat(logdir, "/", pidStr, NULL);
    log_fd = open(log_path, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    
    if(log_fd == -1)
    {
        g_printerr("Cannot write logfile for job id: %d\n", pid);
        org_nixos_disnix_disnix_emit_failure(object, pid);
    }
    
    g_free(log_path);
    return log_fd;
}

void print_paths(int fd, gchar **derivation)
{
    unsigned int i;
    
    for(i = 0; i < g_strv_length(derivation); i++)
        dprintf(fd, "%s ", derivation[i]);
}
