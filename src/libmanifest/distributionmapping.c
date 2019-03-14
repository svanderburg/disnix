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

#include "distributionmapping.h"
#include <xmlutil.h>

static DistributionItem *create_distribution_item_from_dict(GHashTable *table)
{
    DistributionItem *item = (DistributionItem*)g_malloc(sizeof(DistributionItem));
    item->profile = g_hash_table_lookup(table, "profile");
    item->target = g_hash_table_lookup(table, "target");
    return item;
}

static gpointer parse_distribution_item(xmlNodePtr element)
{
    GHashTable *table = parse_dictionary(element, parse_value);
    DistributionItem *item = create_distribution_item_from_dict(table);
    g_hash_table_destroy(table);
    return item;
}

GPtrArray *parse_distribution(xmlNodePtr element)
{
    return parse_list(element, "mapping", parse_distribution_item);
}

void delete_distribution_array(GPtrArray *distribution_array)
{
    if(distribution_array != NULL)
    {
        unsigned int i;

        for(i = 0; i < distribution_array->len; i++)
        {
            DistributionItem* item = g_ptr_array_index(distribution_array, i);

            g_free(item->profile);
            g_free(item->target);
            g_free(item);
        }

        g_ptr_array_free(distribution_array, TRUE);
    }
}

void print_distribution_array(const GPtrArray *distribution_array)
{
    unsigned int i;

    for(i = 0; i < distribution_array->len; i++)
    {
        DistributionItem* item = g_ptr_array_index(distribution_array, i);
        g_print("profile: %s\n", item->profile);
        g_print("target: %s\n\n", item->target);
    }
}

int check_distribution_array(const GPtrArray *distribution_array)
{
    if(distribution_array == NULL)
        return TRUE;
    else
    {
        unsigned int i;

        for(i = 0; i < distribution_array->len; i++)
        {
            DistributionItem *item = g_ptr_array_index(distribution_array, i);
            if(item->profile == NULL || item->target == NULL)
                return FALSE;
        }

        return TRUE;
    }
}
