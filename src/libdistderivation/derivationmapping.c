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

#include "derivationmapping.h"
#include <xmlutil.h>

static DerivationItem *create_derivation_item_from_dict(GHashTable *table)
{
    DerivationItem *item = (DerivationItem*)g_malloc(sizeof(DerivationItem));
    item->derivation = g_hash_table_lookup(table, "derivation");
    item->target = g_hash_table_lookup(table, "target");
    item->result = NULL;
    return item;
}

static gpointer parse_derivation_item(xmlNodePtr element)
{
    GHashTable *table = parse_dictionary(element, parse_value);
    DerivationItem *item = create_derivation_item_from_dict(table);
    g_hash_table_destroy(table);
    return item;
}

GPtrArray *parse_build(xmlNodePtr element)
{
    return parse_list(element, "mapping", parse_derivation_item);
}

void delete_derivation_array(GPtrArray *derivation_array)
{
    if(derivation_array != NULL)
    {
        unsigned int i;
    
        for(i = 0; i < derivation_array->len; i++)
        {
            DerivationItem *item = g_ptr_array_index(derivation_array, i);
            free(item->derivation);
            g_free(item->target);
            g_strfreev(item->result);
            g_free(item);
        }
    
        g_ptr_array_free(derivation_array, TRUE);
    }
}

int check_derivation_array(const GPtrArray *derivation_array)
{
    unsigned int i;

    for(i = 0; i < derivation_array->len; i++)
    {
        DerivationItem *item = g_ptr_array_index(derivation_array, i);

        if(item->derivation == NULL || item->target == NULL)
        {
            /* Check if all mandatory properties have been provided */
            g_printerr("A mandatory property seems to be missing. Have you provided a correct\n");
            g_printerr("distributed derivation file?\n");
            return FALSE;
        }
    }

    return TRUE;
}
