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
#include <nixxml-gptrarray.h>

GPtrArray *parse_interdependency_mapping_array(xmlNodePtr element, void *userdata)
{
    GPtrArray *return_array = NixXML_parse_g_ptr_array(element, "mapping", userdata, parse_interdependency_mapping);
    g_ptr_array_sort(return_array, (GCompareFunc)compare_interdependency_mappings);
    return return_array;
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

int compare_interdependency_mapping_arrays(const GPtrArray *interdependency_mapping_array1, const GPtrArray *interdependency_mapping_array2)
{
    if(interdependency_mapping_array1->len == interdependency_mapping_array2->len)
    {
        unsigned int i;

        for(i = 0; i < interdependency_mapping_array1->len; i++)
        {
            InterDependencyMapping *mapping = g_ptr_array_index(interdependency_mapping_array1, i);
            if(find_interdependency_mapping(interdependency_mapping_array2, mapping) == NULL)
                return FALSE;
        }

        return TRUE;
    }
    else
        return FALSE;
}

void print_interdependency_mapping_array_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    NixXML_print_g_ptr_array_nix(file, value, indent_level, userdata, print_interdependency_mapping_nix);
}

void print_interdependency_mapping_array_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_g_ptr_array_xml(file, (const GPtrArray*)value, "mapping", indent_level, NULL, userdata, print_interdependency_mapping_xml);
}
