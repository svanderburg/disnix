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

#include "signaling.h"
#include <stdio.h>
#include <sys/stat.h>
#include <procreact_pid.h>

/* Boolean signaling infrastructure */

typedef struct
{
    OrgNixosDisnixDisnix *object;
    gint jid;
    int log_fd;
    pid_t pid;
}
SignalBooleanResultThreadData;

static gpointer signal_boolean_result_thread_func(gpointer data)
{
    ProcReact_Status status;
    SignalBooleanResultThreadData *boolean_data = (SignalBooleanResultThreadData*)data;
    int result = procreact_wait_for_boolean(boolean_data->pid, &status);
    
    if(status == PROCREACT_STATUS_OK && result)
        org_nixos_disnix_disnix_emit_finish(boolean_data->object, boolean_data->jid);
    else
        org_nixos_disnix_disnix_emit_failure(boolean_data->object, boolean_data->jid);
    
    /* Cleanup */
    g_free(boolean_data);
    close(boolean_data->log_fd);
    
    return NULL;
}

void signal_boolean_result(pid_t pid, OrgNixosDisnixDisnix *object, gint jid, int log_fd)
{
    GThread *thread;
    SignalBooleanResultThreadData *data = (SignalBooleanResultThreadData*)g_malloc(sizeof(SignalBooleanResultThreadData));
    
    data->object = object;
    data->jid = jid;
    data->log_fd = log_fd;
    data->pid = pid;
    
    thread = g_thread_new("evaluate-boolean", signal_boolean_result_thread_func, data);
    g_thread_unref(thread);
}

/* String vector signaling infrastructure */

typedef struct
{
    OrgNixosDisnixDisnix *object;
    gint jid;
    int log_fd;
    ProcReact_Future future;
}
SignalStrvResultThreadData;

static gpointer signal_strv_result_thread_func(gpointer data)
{
    SignalStrvResultThreadData *strv_data = (SignalStrvResultThreadData*)data;
    ProcReact_Status status;
    char **result = procreact_future_get(&strv_data->future, &status);
    
    if(status != PROCREACT_STATUS_OK || result == NULL)
        org_nixos_disnix_disnix_emit_failure(strv_data->object, strv_data->jid);
    else
    {
        org_nixos_disnix_disnix_emit_success(strv_data->object, strv_data->jid, (const gchar**)result);
        procreact_free_string_array(result);
    }
    
    /* Cleanup */
    g_free(strv_data);
    close(strv_data->log_fd);
    
    return NULL;
}

void signal_strv_result(ProcReact_Future future, OrgNixosDisnixDisnix *object, gint jid, int log_fd)
{
    GThread *thread;
    SignalStrvResultThreadData *data = (SignalStrvResultThreadData*)g_malloc(sizeof(SignalStrvResultThreadData));
    
    data->object = object;
    data->jid = jid;
    data->log_fd = log_fd;
    data->future = future;
    
    thread = g_thread_new("evaluate-strv", signal_strv_result_thread_func, data);
    g_thread_unref(thread);
}

/* Temp file signaling infrastructure */

typedef struct
{
    OrgNixosDisnixDisnix *object;
    gint jid;
    int log_fd;
    pid_t pid;
    gchar *tempfilename;
    int temp_fd;
}
SignalTempFileResultThreadData;

static gpointer evaluate_tempfile_process_thread_func(gpointer data)
{
    SignalTempFileResultThreadData *tempfile_data = (SignalTempFileResultThreadData*)data;
    ProcReact_Status status;
    int result = procreact_wait_for_boolean(tempfile_data->pid, &status);

    if(status == PROCREACT_STATUS_OK && result)
    {
        const gchar *tempfilepaths[] = { tempfile_data->tempfilename, NULL };
        
        if(fchmod(tempfile_data->temp_fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) == 1)
        {
            dprintf(tempfile_data->log_fd, "Cannot change permissions of tempfile: %s\n", tempfile_data->tempfilename);
            org_nixos_disnix_disnix_emit_failure(tempfile_data->object, tempfile_data->jid);
        }
        else
            org_nixos_disnix_disnix_emit_success(tempfile_data->object, tempfile_data->jid, tempfilepaths);
    }
    
    g_free(tempfile_data->tempfilename);
    
    /* Cleanup */
    close(tempfile_data->log_fd);
    close(tempfile_data->temp_fd);
    
    return NULL;
}

void signal_tempfile_result(pid_t pid, gchar *tempfilename, int temp_fd, OrgNixosDisnixDisnix *object, gint jid, int log_fd)
{
    GThread *thread;
    SignalTempFileResultThreadData *data = (SignalTempFileResultThreadData*)g_malloc(sizeof(SignalTempFileResultThreadData));
    
    data->object = object;
    data->jid = jid;
    data->log_fd = log_fd;
    data->pid = pid;
    data->tempfilename = tempfilename;
    data->temp_fd = temp_fd;
    
    thread = g_thread_new("evaluate-tempfile", evaluate_tempfile_process_thread_func, data);
    g_thread_unref(thread);
}
