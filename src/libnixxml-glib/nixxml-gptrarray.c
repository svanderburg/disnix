/*
 * libnixxml - GLib integration with libnixxml
 * Copyright (C) 2019  Sander van der Burg
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

#include "nixxml-gptrarray.h"
#include <stdlib.h>

void *NixXML_create_g_ptr_array(xmlNodePtr element, void *userdata)
{
    return g_ptr_array_new();
}

void NixXML_add_value_to_g_ptr_array(void *list, void *value, void *userdata)
{
    GPtrArray *array = (GPtrArray*)list;
    g_ptr_array_add(array, value);
}

void *NixXML_finalize_g_ptr_array(void *list, void *userdata)
{
    return list;
}

void NixXML_delete_g_ptr_array(GPtrArray *array, NixXML_DeletePtrArrayElementFunc delete_element)
{
    if(array != NULL)
    {
        unsigned int i;

        for(i = 0; i < array->len; i++)
        {
            gpointer element = g_ptr_array_index(array, i);
            delete_element(element);
        }

        g_ptr_array_free(array, TRUE);
    }
}

void NixXML_delete_g_values_array(GPtrArray *array)
{
    NixXML_delete_g_ptr_array(array, free);
}

void NixXML_print_g_ptr_array_elements_nix(FILE *file, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    unsigned int i;
    const GPtrArray *array = (const GPtrArray*)value;

    for(i = 0 ; i < array->len; i++)
    {
        gpointer element = g_ptr_array_index(array, i);
        NixXML_print_list_element_nix(file, element, indent_level, userdata, print_value);
    }
}

void NixXML_print_g_ptr_array_nix(FILE *file, const GPtrArray *array, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    NixXML_print_list_nix(file, array, indent_level, userdata, NixXML_print_g_ptr_array_elements_nix, print_value);
}

void NixXML_print_g_ptr_array_elements_xml(FILE *file, const char *child_element_name, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    const GPtrArray *array = (const GPtrArray*)value;
    unsigned int i;

    for(i = 0; i < array->len; i++)
    {
        gpointer element = g_ptr_array_index(array, i);
        NixXML_print_list_element_xml(file, child_element_name, element, indent_level, type_property_name, userdata, print_value);
    }
}

void NixXML_print_g_ptr_array_xml(FILE *file, const GPtrArray *array, const char *child_element_name, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    NixXML_print_list_xml(file, array, child_element_name, indent_level, type_property_name, userdata, NixXML_print_g_ptr_array_elements_xml, print_value);
}

void *NixXML_parse_g_ptr_array(xmlNodePtr element, const char *child_element_name, void *userdata, NixXML_ParseObjectFunc parse_object)
{
    return NixXML_parse_list(element, child_element_name, userdata, NixXML_create_g_ptr_array, NixXML_add_value_to_g_ptr_array, parse_object, NixXML_finalize_g_ptr_array);
}
