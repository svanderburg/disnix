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

#include "interface.h"
#include <nixxml-parse.h>

static void *create_interface(xmlNodePtr element, void *userdata)
{
    return g_malloc0(sizeof(Interface));
}

static void insert_interface_attributes(void *table, const xmlChar *key, void *value, void *userdata)
{
    Interface *interface = (Interface*)table;

    if(xmlStrcmp(key, (xmlChar*) "targetAddress") == 0)
        interface->target_address = value;
    else if(xmlStrcmp(key, (xmlChar*) "clientInterface") == 0)
        interface->client_interface = value;
    else
        xmlFree(value);
}

void *parse_interface(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_simple_attrset(element, userdata, create_interface, NixXML_parse_value, insert_interface_attributes);
}

void delete_interface(Interface *interface)
{
    g_free(interface->target_address);
    g_free(interface->client_interface);
    g_free(interface);
}

int check_interface(const Interface *interface)
{
    if(interface->target_address == NULL)
    {
        g_printerr("interface.targetAddress is not set!\n");
        return FALSE;
    }
    else if(interface->client_interface == NULL)
    {
        g_printerr("interface.clientInterface is not set!\n");
        return FALSE;
    }

    return TRUE;
}
