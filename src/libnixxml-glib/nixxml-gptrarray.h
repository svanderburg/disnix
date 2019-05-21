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

#ifndef __NIXXML_GPTRARRAY_H
#define __NIXXML_GPTRARRAY_H
#include <libxml/parser.h>
#include <glib.h>
#include "nixxml-parse.h"
#include "nixxml-print-nix.h"
#include "nixxml-print-xml.h"

typedef void (*NixXML_DeletePtrArrayElementFunc) (void *element);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates a new pointer array by parsing the sub elements of a provided
 * XML element.
 *
 * @param element XML element to parse.
 * @param userdata Arbitrary user data that is propagated to all parse functions
 * @return A data structure that encapsulates a pointer array. It must be finalized to become a usable pointer array.
 */
void *NixXML_create_g_ptr_array(xmlNodePtr element, void *userdata);

/**
 * Adds a value to a non-finalized pointer array.
 *
 * @param list A non-finalized pointer array struct
 * @param value Pointer to a value to add
 * @param userdata Arbitrary user data that is propagated to all parse functions
 */
void NixXML_add_value_to_g_ptr_array(void *list, void *value, void *userdata);

/**
 * Finalizes the pointer array (by discarding obsolete resources) and makes it
 * usable to be consumed by the application.
 *
 * @param list A non-finalized pointer array struct
 * @param userdata Arbitrary user data that is propagated to all parse functions
 * @return A GPtrArray
 */
void *NixXML_finalize_g_ptr_array(void *list, void *userdata);

/**
 * Deletes a GPtrArray including its contents
 *
 * @param array GPtrArray array to delete
 * @param delete_element Pointer to a function that deletes each element
 */
void NixXML_delete_g_ptr_array(GPtrArray *array, NixXML_DeletePtrArrayElementFunc delete_element);

/**
 * Deletes a pointer array and the values it refers to.by executing free()
 *
 * @param array GPtrArray to delete
 */
void NixXML_delete_g_values_array(GPtrArray *array);

/**
 * Prints a Nix representation of all elements in the array.
 *
 * @param file File descriptor to write to
 * @param value Pointer array
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 * @param print_value Pointer to a function that prints an element value
 */
void NixXML_print_g_ptr_array_elements_nix(FILE *file, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value);

/**
 * Prints a Nix representation of a list from a GPtrArray.
 *
 * @param file File descriptor to write to
 * @param array GPtrArray
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 * @param print_value Pointer to a function that prints an element value
 */
void NixXML_print_g_ptr_array_nix(FILE *file, const GPtrArray *array, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value);

/**
 * Prints an XML representation of all elements in the array.
 *
 * @param file File descriptor to write to
 * @param child_element_name Name of each list child element
 * @param value GPtrArray
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 * @param print_value Pointer to a function that prints an element value
 */
void NixXML_print_g_ptr_array_elements_xml(FILE *file, const char *child_element_name, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value);

/**
 * Prints an XML representation of a list from a GPtrArray.
 *
 * @param file File descriptor to write to
 * @param array GPtrArray
 * @param child_element_name Name of each list child element
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 * @param print_value Pointer to a function that prints an element value
 */
void NixXML_print_g_ptr_array_xml(FILE *file, const GPtrArray *array, const char *child_element_name, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value);

/**
 * Parses a pointer array from the sub elements of a provided XML element.
 *
 * @param element XML element to parse.
 * @param child_element_name Name of each list child element
 * @param userdata Arbitrary user data that gets propagated to all print functions
 * @param parse_object Pointer to a function that parses the value
 * @return A GPtrArray
 */
void *NixXML_parse_g_ptr_array(xmlNodePtr element, const char *child_element_name, void *userdata, NixXML_ParseObjectFunc parse_object);

#ifdef __cplusplus
}
#endif

#endif
