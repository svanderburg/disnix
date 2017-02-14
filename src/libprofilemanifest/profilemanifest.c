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

typedef enum
{
    LINE_NAME,
    LINE_SERVICE,
    LINE_CONTAINER,
    LINE_TYPE,
    LINE_KEY,
    LINE_STATEFUL,
    LINE_DEPENDS_ON
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
            g_free(entry);
        }
    }
    
    g_ptr_array_free(profile_manifest_array, TRUE);
}
