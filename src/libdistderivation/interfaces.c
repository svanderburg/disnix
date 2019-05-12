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

#include "interfaces.h"
#include <stdlib.h>
#include <nixxml-ghashtable.h>
#include <nixxml-gptrarray.h>

static gint compare_interface(const Interface **l, const Interface **r)
{
    const Interface *left = *l;
    const Interface *right = *r;

    return g_strcmp0(left->target, right->target);
}

static int compare_interface_keys(const char *key, const Interface **r)
{
    const Interface *right = *r;
    return g_strcmp0(key, right->target);
}

static void *create_interface(xmlNodePtr element, void *userdata)
{
    return g_malloc0(sizeof(Interface));
}

static void insert_interface_attributes(void *table, const xmlChar *key, void *value, void *userdata)
{
    Interface *interface = (Interface*)table;

    if(xmlStrcmp(key, (xmlChar*) "target") == 0)
        interface->target = value;
    else if(xmlStrcmp(key, (xmlChar*) "clientInterface") == 0)
        interface->clientInterface = value;
    else
        xmlFree(value);
}

static gpointer parse_interface(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_simple_attrset(element, userdata, create_interface, NixXML_parse_value, insert_interface_attributes);
}

GPtrArray *parse_interfaces(xmlNodePtr element)
{
    GPtrArray *interface_array = NixXML_parse_g_ptr_array(element, "interface", NULL, parse_interface);
    g_ptr_array_sort(interface_array, (GCompareFunc)compare_interface);
    return interface_array;
}

static void delete_interface(Interface *interface)
{
    g_free(interface->target);
    g_free(interface->clientInterface);
    g_free(interface);
}

void delete_interface_array(GPtrArray *interface_array)
{
    if(interface_array != NULL)
    {
        unsigned int i;

        for(i = 0; i < interface_array->len; i++)
        {
            Interface *interface = g_ptr_array_index(interface_array, i);
            delete_interface(interface);
        }

        g_ptr_array_free(interface_array, TRUE);
    }
}

int check_interface_array(const GPtrArray *interface_array)
{
    unsigned int i;

    for(i = 0; i < interface_array->len; i++)
    {
        Interface *interface = g_ptr_array_index(interface_array, i);
        if(interface->target == NULL || interface->clientInterface == NULL)
            return FALSE;
    }

    return TRUE;
}

Interface *find_interface(const GPtrArray *interface_array, const gchar *key)
{
    Interface **ret = bsearch(key, interface_array->pdata, interface_array->len, sizeof(gpointer), (int (*)(const void *, const void *)) compare_interface_keys);

    if(ret == NULL)
        return NULL;
    else
        return *ret;
}
