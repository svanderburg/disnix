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

#include "hashtable-util.h"
#include <libxml/parser.h>

int compare_hash_tables(GHashTable *hash_table1, GHashTable *hash_table2, int (*compare_function) (const gpointer left, const gpointer right))
{
    if(g_hash_table_size(hash_table1) == g_hash_table_size(hash_table2))
    {
        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init(&iter, hash_table1);
        while(g_hash_table_iter_next(&iter, &key, &value))
        {
            gpointer value2;

            if((value2 = g_hash_table_lookup(hash_table2, key)) == NULL)
                return FALSE;
            else
            {
                if(!compare_function(value, value2))
                    return FALSE;
            }
        }

        return TRUE;
    }
    else
        return FALSE;
}

int compare_xml_strings(const gpointer left, const gpointer right)
{
    return (xmlStrcmp((const xmlChar*)left, (const xmlChar*)right) == 0);
}

int compare_property_tables(GHashTable *property_table1, GHashTable *property_table2)
{
    return compare_hash_tables(property_table1, property_table2, compare_xml_strings);
}

int check_value_is_not_null(const gpointer value)
{
    return (value != NULL);
}

int check_hash_table(GHashTable *hash_table, CheckFunction check_function)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, hash_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        if(!check_function(value))
            return FALSE;
    }

    return TRUE;
}

int check_property_table(GHashTable *property_table)
{
    return check_hash_table(property_table, check_value_is_not_null);
}

void delete_xml_value(gpointer key, gpointer value, gpointer user_data)
{
    xmlFree(value);
}
