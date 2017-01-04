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

#include "profilemanifest.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "state-management.h"
#include "procreact_pid.h"

extern char *tmpdir;

static int unlock_services(int log_fd, GPtrArray *profile_manifest_array)
{
    unsigned int i;
    int exit_status = TRUE;
    
    for(i = 0; i < profile_manifest_array->len; i++)
    {
        ProfileManifestEntry *entry = g_ptr_array_index(profile_manifest_array, i);
        pid_t pid;
        ProcReact_Status status;
        int result;
        
        dprintf(log_fd, "Notifying unlock on %s: of type: %s in container: %s\n", entry->derivation, entry->type, entry->container);
        pid = statemgmt_lock_component(entry->type, entry->container, entry->derivation, log_fd, log_fd);
        result = procreact_wait_for_boolean(pid, &status);
        
        if(status != PROCREACT_STATUS_OK || !result)
        {
            dprintf(log_fd, "Cannot unlock service!\n");
            exit_status = FALSE;
        }
    }
    
    return exit_status;
}

static int lock_services(int log_fd, GPtrArray *profile_manifest_array)
{
    unsigned int i;
    int exit_status = 0;

    /* Notify all currently running services that we want to acquire a lock */
    for(i = 0; i < profile_manifest_array->len; i++)
    {
        ProfileManifestEntry *entry = g_ptr_array_index(profile_manifest_array, i);
        pid_t pid;
        ProcReact_Status status;
        int result;
    
        dprintf(log_fd, "Notifying lock on %s: of type: %s in container: %s\n", entry->derivation, entry->type, entry->container);
        
        pid = statemgmt_unlock_component(entry->type, entry->container, entry->derivation, log_fd, log_fd);
        result = procreact_wait_for_boolean(pid, &status);
        
        if(status != PROCREACT_STATUS_OK || !result)
        {
            dprintf(log_fd, "Cannot acquire lock!\n");
            exit_status = FALSE;
        }
    }
    
    return exit_status;
}

static gchar *create_lock_filename(void)
{
    return g_strconcat(tmpdir, "/disnix.lock", NULL);
}

static int lock_disnix(int log_fd)
{
    int fd, status;
    gchar *lock_filename = create_lock_filename();

    /* If no lock exists, try to create one */
    if((fd = open(lock_filename, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
    {
        dprintf(log_fd, "Cannot exclusively open the lock file!\n");
        status = FALSE;
    }
    else
    {
        close(fd);
        status = TRUE;
    }

    g_free(lock_filename);
    return status;
}

static int unlock_disnix(int log_fd)
{
    gchar *lock_filename = create_lock_filename();
    int status;
    
    if(unlink(lock_filename) == -1)
    {
        dprintf(log_fd, "There is no lock file!\n");
        status = FALSE;
    }
    else
        status = TRUE;
    
    /* Cleanup */
    g_free(lock_filename);
    
    return status;
}

typedef enum
{
    LINE_DERIVATION,
    LINE_CONTAINER,
    LINE_TYPE
}
LineType;

GPtrArray *create_profile_manifest_array(gchar *profile)
{
    gchar *manifest_file = g_strconcat(LOCALSTATEDIR "/nix/profiles/disnix/", profile, "/manifest", NULL);
    int fd = open(manifest_file, O_RDONLY);
    g_free(manifest_file);
    
    if(fd == -1)
        return g_ptr_array_new(); /* If the manifest does not exist, we have an empty configuration */
    else
    {
        unsigned int i;
        LineType line_type = LINE_DERIVATION;
        ProfileManifestEntry *entry = NULL;
        GPtrArray *profile_manifest_array = g_ptr_array_new();
        
        /* Initialize a string array type composing a string array from the read file */
        ProcReact_Type type = procreact_create_string_array_type('\n');
        ProcReact_StringArrayState *state = (ProcReact_StringArrayState*)type.initialize();
        
        /* Read from the file and compose a string array from it */
        while(type.append(&type, state, fd) > 0);
        
        /* Process the resulting string array and compose a profile manifest array from it */
        if(state->result_length > 1)
        {
            for(i = 0; i < state->result_length - 1; i++)
            {
                char *line = state->result[i];
                
                /* Compose derivation, container, type triples and add them to an array */
                switch(line_type)
                {
                    case LINE_DERIVATION:
                        entry = (ProfileManifestEntry*)g_malloc(sizeof(ProfileManifestEntry));
                        entry->derivation = line;
                        line_type = LINE_CONTAINER;
                        break;
                    case LINE_CONTAINER:
                        entry->container = line;
                        line_type = LINE_TYPE;
                        break;
                    case LINE_TYPE:
                        entry->type = line;
                        g_ptr_array_add(profile_manifest_array, entry);
                        line_type = LINE_DERIVATION;
                        break;
                }
            }
        }
        
        /* Cleanup */
        free(state->result);
        free(state);
        close(fd);
        
        /* We should have the right number of lines */
        if(line_type == LINE_DERIVATION)
            return profile_manifest_array; /* Return the generate profile manifest array */
        else
        {
            /* If not => the manifest is invalid */
            g_free(entry);
            delete_profile_manifest_array(profile_manifest_array);
            return NULL;
        }
    }
}

void delete_profile_manifest_array(GPtrArray *profile_manifest_array)
{
    if(profile_manifest_array != NULL)
    {
        unsigned int i;
    
        for(i = 0; i < profile_manifest_array->len; i++)
        {
            ProfileManifestEntry *entry = g_ptr_array_index(profile_manifest_array, i);
            g_free(entry->derivation);
            g_free(entry->container);
            g_free(entry->type);
        }
    }
    
    g_ptr_array_free(profile_manifest_array, TRUE);
}

int acquire_locks(int log_fd, GPtrArray *profile_manifest_array)
{
    if(lock_services(log_fd, profile_manifest_array) == 0) /* Attempt to acquire locks from the services */
        return lock_disnix(log_fd); /* Finally, lock disnix itself */
    else
    {
        unlock_services(log_fd, profile_manifest_array);
        return FALSE;
    }
}

pid_t acquire_locks_async(int log_fd, GPtrArray *profile_manifest_array)
{
    pid_t pid = fork();
    
    if(pid == 0)
        _exit(!acquire_locks(log_fd, profile_manifest_array));
    
    return pid;
}

int release_locks(int log_fd, GPtrArray *profile_manifest_array)
{
    int status = TRUE;
    
    if(profile_manifest_array == NULL)
    {
        dprintf(log_fd, "Corrupt profile manifest: a service or type is missing!\n");
        status = FALSE;
    }
    else
    {
        if(!unlock_services(log_fd, profile_manifest_array))
        {
            dprintf(log_fd, "Failed to send unlock notification to old services!\n");
            status = FALSE;
        }
    }
    
    if(!unlock_disnix(log_fd))
        status = FALSE; /* There was no lock -> fail */
    
    return status;
}

pid_t release_locks_async(int log_fd, GPtrArray *profile_manifest_array)
{
    pid_t pid = fork();
    
    if(pid == 0)
        _exit(!release_locks(log_fd, profile_manifest_array));
    
    return pid;
}

ProcReact_Future query_derivations(GPtrArray *profile_manifest_array)
{
    ProcReact_Future future = procreact_initialize_future(procreact_create_string_array_type('\n'));
    
    if(future.pid == 0)
    {
        unsigned int i;
        
        for(i = 0; i < profile_manifest_array->len; i++)
        {
            ProfileManifestEntry *entry = g_ptr_array_index(profile_manifest_array, i);
            dprintf(future.fd, "%s\n", entry->derivation);
        }
        
        _exit(0);
    }
    
    return future;
}
