/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2019  Sander van der Burg
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

#include "servicemappingarray.h"
#include <stdlib.h>
#include <unistd.h>
#include <nixxml-ghashtable.h>
#include <nixxml-gptrarray.h>

#define BUFFER_SIZE 1024

gint compare_service_mappings(const ServiceMapping **l, const ServiceMapping **r)
{
    return compare_interdependency_mappings((const InterDependencyMapping **)l, (const InterDependencyMapping **)r);
}

static void *create_service_mapping(xmlNodePtr element, void *userdata)
{
    return g_malloc0(sizeof(ServiceMapping));
}

static void *parse_service_mapping(xmlNodePtr element, void *userdata)
{
    ServiceMapping *mapping = NixXML_parse_simple_attrset(element, userdata, create_service_mapping, NixXML_parse_value, insert_interdependency_mapping_attributes);

    /* Set default values */
    if(mapping->target == NULL)
    {
        char buffer[BUFFER_SIZE]; /* If no target is set, then use the hostname */
        int status = gethostname(buffer, BUFFER_SIZE);

        if(status != -1)
            mapping->target = xmlStrdup((xmlChar*)buffer);
    }

    return mapping;
}

GPtrArray *parse_service_mapping_array(xmlNodePtr element, void *userdata)
{
    GPtrArray *service_mapping_array = NixXML_parse_g_ptr_array(element, "mapping", userdata, parse_service_mapping);

    /* Sort the service mapping array */
    g_ptr_array_sort(service_mapping_array, (GCompareFunc)compare_service_mappings);

    return service_mapping_array;
}

void delete_service_mapping_array(GPtrArray *service_mapping_array)
{
    delete_interdependency_mapping_array(service_mapping_array);
}

int check_service_mapping_array(const GPtrArray *service_mapping_array)
{
    return check_interdependency_mapping_array(service_mapping_array);
}

int compare_service_mapping_arrays(const GPtrArray *service_mapping_array1, const GPtrArray *service_mapping_array2)
{
    return compare_interdependency_mapping_arrays(service_mapping_array1, service_mapping_array2);
}

void print_service_mapping_array_nix(FILE *file, const GPtrArray *service_mapping_array, const int indent_level, void *userdata)
{
    print_interdependency_mapping_array_nix(file, service_mapping_array, indent_level, userdata);
}

void print_service_mapping_array_xml(FILE *file, const GPtrArray *service_mapping_array, const int indent_level, const char *type_property_name, void *userdata)
{
    print_interdependency_mapping_array_xml(file, service_mapping_array, indent_level, type_property_name, userdata);
}

ServiceMapping *find_service_mapping(const GPtrArray *service_mapping_array, const InterDependencyMapping *key)
{
    ServiceMapping **ret = bsearch(&key, service_mapping_array->pdata, service_mapping_array->len, sizeof(gpointer), (int (*)(const void*, const void*)) compare_service_mappings);

    if(ret == NULL)
        return NULL;
    else
        return *ret;
}

GPtrArray *intersect_service_mapping_array(const GPtrArray *left, const GPtrArray *right)
{
    unsigned int i;
    GPtrArray *return_array = g_ptr_array_new();

    if(left->len < right->len)
    {
        for(i = 0; i < left->len; i++)
        {
            ServiceMapping *left_mapping = g_ptr_array_index(left, i);

            if(find_service_mapping(right, (InterDependencyMapping*)left_mapping) != NULL)
                g_ptr_array_add(return_array, left_mapping);
        }
    }
    else
    {
        for(i = 0; i < right->len; i++)
        {
            ServiceMapping *right_mapping = g_ptr_array_index(right, i);

            if(find_service_mapping(left, (InterDependencyMapping*)right_mapping) != NULL)
                g_ptr_array_add(return_array, right_mapping);
        }
    }

    return return_array;
}

GPtrArray *unify_service_mapping_array(GPtrArray *left, GPtrArray *right, const GPtrArray *intersect)
{
    unsigned int i;
    GPtrArray *return_array = g_ptr_array_new();

    /* Create a clone of the left array and mark mappings as activated */

    for(i = 0; i < left->len; i++)
    {
        ServiceMapping *mapping = g_ptr_array_index(left, i);
        mapping->status = SERVICE_MAPPING_ACTIVATED;
        g_ptr_array_add(return_array, mapping);
    }

    /* Append all mappings from the right array which are not in the intersection and mark them as deactivated */

    for(i = 0; i < right->len; i++)
    {
        ServiceMapping *mapping = g_ptr_array_index(right, i);
        mapping->status = SERVICE_MAPPING_DEACTIVATED;

        if(find_service_mapping(intersect, (InterDependencyMapping*)mapping) == NULL)
            g_ptr_array_add(return_array, mapping);
    }

    /* Sort the service mapping array */
    g_ptr_array_sort(return_array, (GCompareFunc)compare_service_mappings);

    /* Return the service mapping array */
    return return_array;
}

GPtrArray *substract_service_mapping_array(const GPtrArray *left, const GPtrArray *right)
{
    unsigned int i;
    GPtrArray *return_array = g_ptr_array_new();

    /* Add all elements of the left array that are not in the right array */
    for(i = 0; i < left->len; i++)
    {
        ServiceMapping *mapping = g_ptr_array_index(left, i);

        if(find_service_mapping(right, (InterDependencyMapping*)mapping) == NULL)
            g_ptr_array_add(return_array, mapping);
    }

    /* Return the service mapping array */
    return return_array;
}
