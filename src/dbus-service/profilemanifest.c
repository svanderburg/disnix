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
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include "state-management.h"

#define BUFFER_SIZE 1024

extern char *tmpdir;

static int unlock_services(int log_fd, GPtrArray *profile_manifest_array)
{
    unsigned int i;
    int exit_status = TRUE;
    
    for(i = 0; i < profile_manifest_array->len; i++)
    {
        ProfileManifestEntry *entry = g_ptr_array_index(profile_manifest_array, i);
        pid_t pid;
        
        dprintf(log_fd, "Notifying unlock on %s: of type: %s in container: %s\n", entry->derivation, entry->type, entry->container);
        pid = statemgmt_lock_component(entry->type, entry->container, entry->derivation, log_fd, log_fd);
        
        if(pid == -1)
        {
            dprintf(log_fd, "Error forking unlock process!\n");
            exit_status = FALSE;
        }
        else if(pid > 0)
        {
            wait(&pid);
        
            if(!WIFEXITED(pid) || WEXITSTATUS(pid) != 0)
            {
                dprintf(log_fd, "Unlock failed!\n");
                exit_status = FALSE;
            }
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
    
        dprintf(log_fd, "Notifying lock on %s: of type: %s in container: %s\n", entry->derivation, entry->type, entry->container);
        
        pid = statemgmt_unlock_component(entry->type, entry->container, entry->derivation, log_fd, log_fd);

        if(pid == -1)
        {
            dprintf(log_fd, "Error forking lock process!\n");
            exit_status = -1;
        }
        else if(pid > 0)
        {
            wait(&pid);

            if(!WIFEXITED(pid) || WEXITSTATUS(pid) != 0)
            {
                dprintf(log_fd, "Lock rejected!\n");
                exit_status = -1;
            }
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
    FILE *fp = fopen(manifest_file, "r");
    g_free(manifest_file);
    
    if(fp == NULL)
        return g_ptr_array_new();
    else
    {
        char line[BUFFER_SIZE];
        LineType line_type = LINE_DERIVATION;
        GPtrArray *profile_manifest_array = g_ptr_array_new();
        ProfileManifestEntry *entry = NULL;

        /* Read the output */

        while(fgets(line, BUFFER_SIZE - 1, fp) != NULL)
        {
            unsigned int line_length = strlen(line);
            
            /* Chop off the linefeed at the end */
            if(line > 0)
                line[line_length - 1] = '\0';
            
            /* Compose derivation and type pairs and add them to an array */
            switch(line_type)
            {
                case LINE_DERIVATION:
                    entry = (ProfileManifestEntry*)g_malloc(sizeof(ProfileManifestEntry));
                    entry->derivation = g_strdup(line);
                    line_type = LINE_CONTAINER;
                    break;
                case LINE_CONTAINER:
                    entry->container = g_strdup(line);
                    line_type = LINE_TYPE;
                    break;
                case LINE_TYPE:
                    entry->type = g_strdup(line);
                    g_ptr_array_add(profile_manifest_array, entry);
                    line_type = LINE_DERIVATION;
                    break;
            }
        }

        fclose(fp);
        
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

gchar **query_derivations(GPtrArray *profile_manifest_array)
{
    gchar **derivations = (gchar**)g_malloc((profile_manifest_array->len + 1) * sizeof(gchar*));
    unsigned int i;
    
    for(i = 0; i < profile_manifest_array->len; i++)
    {
        ProfileManifestEntry *entry = g_ptr_array_index(profile_manifest_array, i);
        derivations[i] = g_strconcat(entry->derivation, "\n", NULL);
    }
    
    derivations[i] = NULL;
    
    return derivations;
}
