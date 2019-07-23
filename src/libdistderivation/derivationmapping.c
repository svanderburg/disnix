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
#include <nixxml-parse.h>

static void *create_derivation_mapping(xmlNodePtr element, void *userdata)
{
    return g_malloc0(sizeof(DerivationMapping));
}

static void insert_derivation_mapping_attributes(void *table, const xmlChar *key, void *value, void *userdata)
{
    DerivationMapping *mapping = (DerivationMapping*)table;

    if(xmlStrcmp(key, (xmlChar*) "derivation") == 0)
        mapping->derivation = value;
    else if(xmlStrcmp(key, (xmlChar*) "interface") == 0)
        mapping->interface = value;
    else
        xmlFree(value);
}

void *parse_derivation_mapping(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_simple_attrset(element, userdata, create_derivation_mapping, NixXML_parse_value, insert_derivation_mapping_attributes);
}

void delete_derivation_mapping(DerivationMapping *mapping)
{
    xmlFree(mapping->derivation);
    xmlFree(mapping->interface);
    g_strfreev(mapping->result);
    g_free(mapping);
}

int check_derivation_mapping(const DerivationMapping *mapping)
{
    if(mapping->derivation == NULL)
    {
        g_printerr("derivationmapping.derivation is not set!\n");
        return FALSE;
    }

    if(mapping->interface == NULL)
    {
        g_printerr("derivationmapping.interface is not set!\n");
        return FALSE;
    }

    return TRUE;
}
