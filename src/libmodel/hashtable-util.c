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

void delete_hash_table(GHashTable *hash_table, DeleteFunction delete_function)
{
    if(hash_table != NULL)
    {
        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init(&iter, hash_table);
        while(g_hash_table_iter_next(&iter, &key, &value))
            delete_function(value);

        g_hash_table_destroy(hash_table);
    }
}

int compare_hash_tables(GHashTable *hash_table1, GHashTable *hash_table2, CompareFunction compare_function)
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

int check_hash_table(GHashTable *hash_table, CheckFunction check_function)
{
    GHashTableIter iter;
    gpointer key, value;
    int status = TRUE;

    g_hash_table_iter_init(&iter, hash_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        if(!check_function(value))
        {
            g_printerr("Hash table value with key: %s is invalid!\n", (gchar*)key);
            status = FALSE;
        }
    }

    return status;
}
