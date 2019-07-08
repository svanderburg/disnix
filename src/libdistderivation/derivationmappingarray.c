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

#include "derivationmappingarray.h"
#include <nixxml-ghashtable.h>
#include <nixxml-gptrarray.h>

static void *create_derivation_item(xmlNodePtr element, void *userdata)
{
    return g_malloc0(sizeof(DerivationItem));
}

static void insert_derivation_item_attributes(void *table, const xmlChar *key, void *value, void *userdata)
{
    DerivationItem *item = (DerivationItem*)table;

    if(xmlStrcmp(key, (xmlChar*) "derivation") == 0)
        item->derivation = value;
    else if(xmlStrcmp(key, (xmlChar*) "interface") == 0)
        item->interface = value;
    else
        xmlFree(value);
}

static gpointer parse_derivation_item(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_simple_attrset(element, userdata, create_derivation_item, NixXML_parse_value, insert_derivation_item_attributes);
}

GPtrArray *parse_derivation_mapping_array(xmlNodePtr element)
{
    return NixXML_parse_g_ptr_array(element, "mapping", NULL, parse_derivation_item);
}

void delete_derivation_mapping_array(GPtrArray *derivation_array)
{
    if(derivation_array != NULL)
    {
        unsigned int i;

        for(i = 0; i < derivation_array->len; i++)
        {
            DerivationItem *item = g_ptr_array_index(derivation_array, i);
            xmlFree(item->derivation);
            xmlFree(item->interface);
            g_strfreev(item->result);
            g_free(item);
        }

        g_ptr_array_free(derivation_array, TRUE);
    }
}

int check_derivation_mapping_array(const GPtrArray *derivation_array)
{
    unsigned int i;

    for(i = 0; i < derivation_array->len; i++)
    {
        DerivationItem *item = g_ptr_array_index(derivation_array, i);

        if(item->derivation == NULL || item->interface == NULL)
        {
            /* Check if all mandatory properties have been provided */
            g_printerr("A mandatory property seems to be missing. Have you provided a correct\n");
            g_printerr("distributed derivation file?\n");
            return FALSE;
        }
    }

    return TRUE;
}
