/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2021  Sander van der Burg
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
#include <nixxml-gptrarray.h>

static void *create_distribution_item(xmlNodePtr element, void *userdata)
{
    return g_malloc0(sizeof(DistributionItem));
}

static void insert_distribution_item_attributes(void *table, const xmlChar *key, void *value, void *userdata)
{
    DistributionItem *item = (DistributionItem*)table;

    if(xmlStrcmp(key, (xmlChar*) "profile") == 0)
        item->profile = value;
    else if(xmlStrcmp(key, (xmlChar*) "target") == 0)
        item->target = value;
    else
        xmlFree(value);
}

static gpointer parse_distribution_item(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_simple_attrset(element, userdata, create_distribution_item, NixXML_parse_value, insert_distribution_item_attributes);
}

GPtrArray *parse_distribution(xmlNodePtr element)
{
    return NixXML_parse_g_ptr_array(element, "mapping", NULL, parse_distribution_item);
}

void delete_distribution_array(GPtrArray *distribution_array)
{
    if(distribution_array != NULL)
    {
        unsigned int i;

        for(i = 0; i < distribution_array->len; i++)
        {
            DistributionItem* item = g_ptr_array_index(distribution_array, i);

            xmlFree(item->profile);
            xmlFree(item->target);
            g_free(item);
        }

        g_ptr_array_free(distribution_array, TRUE);
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
