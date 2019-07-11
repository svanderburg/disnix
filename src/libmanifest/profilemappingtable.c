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

#include "profilemappingtable.h"
#include <nixxml-print-nix.h>
#include <nixxml-ghashtable.h>

GHashTable *parse_profile_mapping_table(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_g_hash_table_verbose(element, "profile", "name", userdata, NixXML_parse_value);
}

void delete_profile_mapping_table(GHashTable *profile_mapping_table)
{
    if(profile_mapping_table != NULL)
    {
        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init(&iter, profile_mapping_table);
        while(g_hash_table_iter_next(&iter, &key, &value))
        {
            xmlChar *target = (xmlChar*)value;
            xmlFree(target);
        }

        g_hash_table_destroy(profile_mapping_table);
    }
}

int check_profile_mapping_table(GHashTable *profile_mapping_table)
{
    if(profile_mapping_table == NULL)
        return TRUE;
    else
    {
        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init(&iter, profile_mapping_table);
        while(g_hash_table_iter_next(&iter, &key, &value))
        {
            xmlChar *target = (xmlChar*)value;
            if(target == NULL)
                return FALSE;
        }

        return TRUE;
    }
}

void print_profile_mapping_table_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    NixXML_print_g_hash_table_nix(file, (GHashTable*)value, indent_level, userdata, NixXML_print_store_path_nix);
}
