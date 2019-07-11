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

#include "interdependencymappingarray.h"
#include <nixxml-parse.h>
#include <nixxml-print-nix.h>
#include <nixxml-gptrarray.h>

gint compare_interdependency_mappings(const InterDependencyMapping **l, const InterDependencyMapping **r)
{
    const InterDependencyMapping *left = *l;
    const InterDependencyMapping *right = *r;

    /* Compare the service keys */
    gint status = xmlStrcmp(left->service, right->service);

    if(status == 0)
    {
        status = xmlStrcmp(left->target, right->target); /* If services are equal then compare the targets */

        if(status == 0)
            return xmlStrcmp(left->container, right->container); /* If targets are equal then compare the containers */
        else
            return status;
    }
    else
        return status;
}

static void *create_interdependency_mapping(xmlNodePtr element, void *userdata)
{
    return g_malloc0(sizeof(InterDependencyMapping));
}

void insert_interdependency_mapping_attributes(void *table, const xmlChar *key, void *value, void *userdata)
{
    InterDependencyMapping *mapping = (InterDependencyMapping*)table;

    if(xmlStrcmp(key, (xmlChar*) "service") == 0)
        mapping->service = value;
    else if(xmlStrcmp(key, (xmlChar*) "target") == 0)
        mapping->target = value;
    else if(xmlStrcmp(key, (xmlChar*) "container") == 0)
        mapping->container = value;
    else
        xmlFree(value);
}

static void *parse_interdependency_mapping(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_simple_attrset(element, userdata, create_interdependency_mapping, NixXML_parse_value, insert_interdependency_mapping_attributes);
}

GPtrArray *parse_interdependency_mapping_array(xmlNodePtr element, void *userdata)
{
    GPtrArray *return_array = NixXML_parse_g_ptr_array(element, "mapping", userdata, parse_interdependency_mapping);
    g_ptr_array_sort(return_array, (GCompareFunc)compare_interdependency_mappings);
    return return_array;
}

static int check_interdependency_mapping(InterDependencyMapping *mapping)
{
    if(mapping->service == NULL || mapping->container == NULL || mapping->target == NULL)
    {
        /* Check if all mandatory properties have been provided */
        g_printerr("A mandatory property seems to be missing. Have you provided a correct\n");
        g_printerr("manifest file?\n");
        return FALSE;
    }
    else
        return TRUE;
}

int check_interdependency_mapping_array(const GPtrArray *interdependency_mapping_array)
{
    if(interdependency_mapping_array == NULL)
        return TRUE;
    else
    {
        unsigned int i;

        for(i = 0; i < interdependency_mapping_array->len; i++)
        {
            InterDependencyMapping *mapping = g_ptr_array_index(interdependency_mapping_array, i);

            if(!check_interdependency_mapping(mapping))
                return FALSE;
        }

        return TRUE;
    }
}

static void delete_interdependency_mapping(InterDependencyMapping *mapping)
{
    xmlFree(mapping->service);
    xmlFree(mapping->container);
    xmlFree(mapping->target);
    g_free(mapping);
}

void delete_interdependency_mapping_array(GPtrArray *interdependency_mapping_array)
{
    if(interdependency_mapping_array != NULL)
    {
        unsigned int i;

        for(i = 0; i < interdependency_mapping_array->len; i++)
        {
            InterDependencyMapping *mapping = g_ptr_array_index(interdependency_mapping_array, i);
            delete_interdependency_mapping(mapping);
        }

        g_ptr_array_free(interdependency_mapping_array, TRUE);
    }
}

InterDependencyMapping *find_interdependency_mapping(const GPtrArray *interdependency_mapping_array, const InterDependencyMapping *key)
{
    InterDependencyMapping **ret = bsearch(&key, interdependency_mapping_array->pdata, interdependency_mapping_array->len, sizeof(gpointer), (int (*)(const void*, const void*)) compare_interdependency_mappings);

    if(ret == NULL)
        return NULL;
    else
        return *ret;
}

static void print_interdependency_mapping_attributes_nix(FILE *file, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    InterDependencyMapping *mapping = (InterDependencyMapping*)value;

    NixXML_print_attribute_nix(file, "service", mapping->service, indent_level, userdata, NixXML_print_string_nix);
    NixXML_print_attribute_nix(file, "container", mapping->container, indent_level, userdata, NixXML_print_string_nix);
    if(mapping->target != NULL)
        NixXML_print_attribute_nix(file, "target", mapping->target, indent_level, userdata, NixXML_print_string_nix);
}

static void print_interdependency_mapping_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    NixXML_print_attrset_nix(file, value, indent_level, userdata, print_interdependency_mapping_attributes_nix, NULL);
}

void print_interdependency_mapping_array_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    NixXML_print_g_ptr_array_nix(file, value, indent_level, userdata, print_interdependency_mapping_nix);
}
