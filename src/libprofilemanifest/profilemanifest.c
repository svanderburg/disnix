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

#include "profilemanifest.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <procreact_types.h>

typedef enum
{
    LINE_NAME,
    LINE_SERVICE,
    LINE_CONTAINER,
    LINE_TYPE,
    LINE_KEY,
    LINE_STATEFUL,
    LINE_DEPENDS_ON,
    LINE_CONNECTS_TO
}
LineType;

GPtrArray *create_profile_manifest_array_from_string_array(char **result)
{
    GPtrArray *profile_manifest_array = g_ptr_array_new();
    LineType line_type = LINE_NAME;
    ProfileManifestEntry *entry = NULL;
    
    if(result != NULL && result[0] != NULL)
    {
        unsigned int count = 0;
        char *line;
        
        while((line = result[count]) != NULL)
        {
            /* Compose profile manifest entries and add them to an array */
            switch(line_type)
            {
                case LINE_NAME:
                    entry = (ProfileManifestEntry*)g_malloc(sizeof(ProfileManifestEntry));
                    entry->name = line;
                    line_type = LINE_SERVICE;
                    break;
                case LINE_SERVICE:
                    entry->service = line;
                    line_type = LINE_CONTAINER;
                    break;
                case LINE_CONTAINER:
                    entry->container = line;
                    line_type = LINE_TYPE;
                    break;
                case LINE_TYPE:
                    entry->type = line;
                    line_type = LINE_KEY;
                    break;
                case LINE_KEY:
                    entry->key = line;
                    line_type = LINE_STATEFUL;
                    break;
                case LINE_STATEFUL:
                    entry->stateful = line;
                    line_type = LINE_DEPENDS_ON;
                    break;
                case LINE_DEPENDS_ON:
                    entry->depends_on = line;
                    line_type = LINE_CONNECTS_TO;
                    break;
                case LINE_CONNECTS_TO:
                    entry->connects_to = line;
                    line_type = LINE_NAME;
                    g_ptr_array_add(profile_manifest_array, entry);
                    break;
            }

            count++;
        }
    }

    /* We should have the right number of lines */
    if(line_type == LINE_NAME)
        return profile_manifest_array; /* Return the generate profile manifest array */
    else
    {
        /* If not => the manifest is invalid */
        g_free(entry);
        delete_profile_manifest_array(profile_manifest_array);
        return NULL;
    }
}

GPtrArray *create_profile_manifest_array_from_file(gchar *manifest_file)
{
    int fd = open(manifest_file, O_RDONLY);
    
    if(fd == -1)
        return g_ptr_array_new(); /* If the manifest does not exist, we have an empty configuration */
    else
    {
        GPtrArray *profile_manifest_array;
        
        /* Initialize a string array type composing a string array from the read file */
        ProcReact_Type type = procreact_create_string_array_type('\n');
        ProcReact_StringArrayState *state = (ProcReact_StringArrayState*)type.initialize();
        
        /* Read from the file and compose a string array from it */
        while(type.append(&type, state, fd) > 0);
        
        /* Append NULL termination */
        state->result = (char**)realloc(state->result, (state->result_length + 1) * sizeof(char*));
        state->result[state->result_length] = NULL;
        
        /* Parse the array for manifest data */
        profile_manifest_array = create_profile_manifest_array_from_string_array(state->result);
        
        /* Cleanup */
        free(state->result);
        free(state);
        close(fd);
        
        /* Returns the corresponding array */
        return profile_manifest_array;
    }
}

GPtrArray *create_profile_manifest_array_from_current_deployment(gchar *localstatedir, gchar *profile)
{
    gchar *manifest_file = g_strconcat(localstatedir, "/nix/profiles/disnix/", profile, "/manifest", NULL);
    GPtrArray *profile_manifest_array = create_profile_manifest_array_from_file(manifest_file);
    g_free(manifest_file);
    return profile_manifest_array;
}

void delete_profile_manifest_array(GPtrArray *profile_manifest_array)
{
    if(profile_manifest_array != NULL)
    {
        unsigned int i;
    
        for(i = 0; i < profile_manifest_array->len; i++)
        {
            ProfileManifestEntry *entry = g_ptr_array_index(profile_manifest_array, i);
            g_free(entry->name);
            g_free(entry->service);
            g_free(entry->container);
            g_free(entry->type);
            g_free(entry->key);
            g_free(entry->stateful);
            g_free(entry->depends_on);
            g_free(entry->connects_to);
            g_free(entry);
        }
    }
    
    g_ptr_array_free(profile_manifest_array, TRUE);
}

void print_services_in_profile_manifest_array(const GPtrArray *profile_manifest_array)
{
    unsigned int i;
    
    for(i = 0; i < profile_manifest_array->len; i++)
    {
        ProfileManifestEntry *entry = g_ptr_array_index(profile_manifest_array, i);
        g_print("%s\n", entry->service);
    }
}

static gint compare_profile_manifest_entry(gconstpointer l, gconstpointer r)
{
    const ProfileManifestEntry *left = *((ProfileManifestEntry **)l);
    const ProfileManifestEntry *right = *((ProfileManifestEntry **)r);
    
    int result = g_strcmp0(left->container, right->container);
    
    if(result == 0)
        return g_strcmp0(left->name, right->name);
    else
        return result;
}

void print_services_per_container_in_profile_manifest_array(GPtrArray *profile_manifest_array)
{
    unsigned int i;
    gchar *last_container = "";
    
    g_ptr_array_sort(profile_manifest_array, compare_profile_manifest_entry);
    
    for(i = 0; i < profile_manifest_array->len; i++)
    {
        ProfileManifestEntry *entry = g_ptr_array_index(profile_manifest_array, i);
        
        if(g_strcmp0(entry->container, last_container) != 0)
        {
            last_container = entry->container;
            g_print("  Container: %s\n", last_container);
        }
        
        g_print("    %s\n", entry->service);
    }
}

void print_text_from_profile_manifest_array(const GPtrArray *profile_manifest_array, int fd)
{
    unsigned int i;

    for(i = 0; i < profile_manifest_array->len; i++)
    {
        ProfileManifestEntry *entry = g_ptr_array_index(profile_manifest_array, i);
        dprintf(fd, "%s\n", entry->name);
        dprintf(fd, "%s\n", entry->service);
        dprintf(fd, "%s\n", entry->container);
        dprintf(fd, "%s\n", entry->type);
        dprintf(fd, "%s\n", entry->key);
        dprintf(fd, "%s\n", entry->stateful);
        dprintf(fd, "%s\n", entry->depends_on);
        dprintf(fd, "%s\n", entry->connects_to);
    }
}

void print_nix_expression_from_profile_manifest_array(const GPtrArray *profile_manifest_array)
{
    unsigned int i;

    g_print("[\n");

    for(i = 0; i < profile_manifest_array->len; i++)
    {
        ProfileManifestEntry *entry = g_ptr_array_index(profile_manifest_array, i);
        g_print("      { name = \"%s\";\n", entry->name);
        g_print("        service = builtins.storePath %s;\n", entry->service);
        g_print("        container = \"%s\";\n", entry->container);
        g_print("        type = \"%s\";\n", entry->type);
        g_print("        _key = \"%s\";\n", entry->key);
        g_print("        stateful = %s;\n", entry->stateful);
        g_print("        dependsOn = %s;\n", entry->depends_on);
        g_print("        connectsTo = %s;\n", entry->connects_to);
        g_print("      }\n");
    }

    g_print("    ]");
}
