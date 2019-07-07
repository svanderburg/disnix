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

#include "distributionmappingtable.h"
#include <nixxml-ghashtable.h>

GHashTable *parse_distribution_table(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_g_hash_table_verbose(element, "distribution", "name", userdata, NixXML_parse_value);
}

void delete_distribution_table(GHashTable *distribution_table)
{
    if(distribution_table != NULL)
    {
        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init(&iter, distribution_table);
        while(g_hash_table_iter_next(&iter, &key, &value))
        {
            xmlChar *target = (xmlChar*)value;
            xmlFree(target);
        }

        g_hash_table_destroy(distribution_table);
    }
}

int check_distribution_table(GHashTable *distribution_table)
{
    if(distribution_table == NULL)
        return TRUE;
    else
    {
        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init(&iter, distribution_table);
        while(g_hash_table_iter_next(&iter, &key, &value))
        {
            xmlChar *target = (xmlChar*)value;
            if(target == NULL)
                return FALSE;
        }

        return TRUE;
    }
}
