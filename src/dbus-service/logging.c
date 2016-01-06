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

static gchar *compose_line_from_buffer(char *buf, unsigned int start_offset, unsigned int pos)
{
    unsigned int line_size = pos - start_offset + 1;
    gchar *line = (gchar*)g_malloc((line_size + 1) * sizeof(gchar));
    strncpy(line, buf + start_offset, line_size);
    line[line_size] = '\0';
    
    return line;
}

static gchar **append_line_to_lines_array(gchar *line, gchar **lines, unsigned int *lines_length)
{
    *lines_length = *lines_length + 1;
    lines = (gchar**)g_realloc(lines, (*lines_length + 1) * sizeof(gchar*));
    lines[*lines_length - 1] = line;
    lines[*lines_length] = NULL;
    
    return lines;
}

static gchar **append_or_concatenate_buffer(gchar **lines, char *buf, unsigned int start_offset, unsigned int pos, unsigned int *lines_length)
{
    gchar *line = compose_line_from_buffer(buf, start_offset, pos);
    
    if(*lines_length == 0)
        lines = append_line_to_lines_array(line, lines, lines_length); /* If the lines vector is empty => simply add the line */
    else
    {
        gchar *last_line = lines[*lines_length - 1]; /* Take the last line */
        size_t last_line_length = strlen(last_line);
        
        if(last_line_length > 0 && last_line[last_line_length - 1] == '\n') /* If the last line ends with a linefeed, add a new entry to the vector */
            lines = append_line_to_lines_array(line, lines, lines_length);
        else
        {
            /* Otherwise, concatenate buffer to the last line */
            lines[*lines_length - 1] = g_strconcat(last_line, line, NULL);
            
            /* Cleanup obsolete old line and the composed line */
            g_free(last_line);
            g_free(line);
        }
    }
    
    return lines;
}

gchar **update_lines_vector(gchar **lines, char *buf)
{
    unsigned int i;
    unsigned int start_offset = 0;
    unsigned int lines_length;
    
    if(lines == NULL)
        lines_length = 0;
    else
        lines_length = g_strv_length(lines);
    
    /* Check the buffer for lines */
    for(i = 0; i < strlen(buf); i++)
    {
        /* If a linefeed is found append the line to the lines vector */
        if(buf[i] == '\n')
        {
            lines = append_or_concatenate_buffer(lines, buf, start_offset, i, &lines_length);
            start_offset = i + 1;
        }
    }
    
    /* If there is trailing stuff, append it to the lines vector */
    if(start_offset < i)
        lines = append_or_concatenate_buffer(lines, buf, start_offset, i, &lines_length);
    
    /* Return modified lines vector */
    return lines;
}

int open_log_file(const gint pid)
{
    gchar pidStr[15];
    gchar *log_path;
    int log_fd;
    
    sprintf(pidStr, "%d", pid);
    
    mkdir(logdir, 0755);
    log_path = g_strconcat(logdir, "/", pidStr, NULL);
    log_fd = open(log_path, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    
    g_free(log_path);
    return log_fd;
}

void print_paths(int fd, gchar **derivation)
{
    unsigned int i;
    
    for(i = 0; i < g_strv_length(derivation); i++)
        dprintf(fd, "%s ", derivation[i]);
}
