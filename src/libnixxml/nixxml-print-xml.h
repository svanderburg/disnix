/*
 * Copyright (c) 2019 Sander van der Burg
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so, 
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __NIXXML_PRINT_XML_H
#define __NIXXML_PRINT_XML_H
#include <stdio.h>
#include "nixxml-print.h"

/**
 * Prints an XML representation of a value.
 *
 * @param file File descriptor to write to
 * @param value Pointer to a data structure to print
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
typedef void (*NixXML_PrintXMLValueFunc) (FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata);

/**
 * Prints an XML representation of the members of a list.
 *
 * @param file File descriptor to write to
 * @param child_element_name Name of each attribute child element
 * @param value Pointer to a data structure to print
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 * @param print_value Pointer to a function that prints an attribute value
 */
typedef void (*NixXML_PrintXMLListMembersFunc) (FILE *file, const char *child_element_name, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value);

/**
 * Prints a simple XML representation of the members of an attribute set.
 *
 * @param file File descriptor to write to
 * @param value Pointer to a data structure to print
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 * @param print_value Pointer to a function that prints an attribute value
 */
typedef void (*NixXML_PrintSimpleXMLMembersFunc) (FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value);

/**
 * Prints a verbose XML representation of the members of an attribute set.
 *
 * @param file File descriptor to write to
 * @param value Pointer to a data structure to print
 * @param child_element_name Name of each attribute child element
 * @param name_property_name Name of the name property
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 * @param print_value Pointer to a function that prints an attribute value
 */
typedef void (*NixXML_PrintVerboseXMLMembersFunc) (FILE *file, const void *value, const char *child_element_name, const char *name_property_name, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Prints an opening XML tag.
 *
 * @param file File descriptor to write to
 * @param root_element_name Name of the root element
 */
void NixXML_print_open_root_tag(FILE *file, const char *root_element_name);

/**
 * Prints an opening XML tag for a verbose attribute.
 *
 * @param file File descriptor to write to
 * @param root_element_name Name of the root element
 * @param name_property_name Name of the name property
 * @param name Value that the name property has
 */
void NixXML_print_open_verbose_attr_tag(FILE *file, const char *root_element_name, const char *name_property_name, const char *name);

/**
 * Prints a closing XML tag.
 *
 * @param file File descriptor to write to
 * @param root_element_name Name of the root element
 */
void NixXML_print_close_root_tag(FILE *file, const char *root_element_name);

/**
 * Prints a string and escapes it, if required.
 *
 * @param file File descriptor to write to
 * @param value Pointer to the first character of a string
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void NixXML_print_string_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata);

/**
 * Prints a string and annotates it with an integer type.
 *
 * @param file File descriptor to write to
 * @param value Pointer to the first character of a string
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void NixXML_print_string_as_int_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata);

/**
 * Prints a string and annotates it with a float type.
 *
 * @param file File descriptor to write to
 * @param value Pointer to the first character of a string
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void NixXML_print_string_as_float_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata);

/**
 * Prints a string and annotates it with a bool type.
 *
 * @param file File descriptor to write to
 * @param value Pointer to the first character of a string
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void NixXML_print_string_as_bool_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata);

/**
 * Prints an integer.
 *
 * @param file File descriptor to write to
 * @param value Pointer to an integer
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void NixXML_print_int_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata);

/**
 * Prints an XML representation of a list element.
 *
 * @param file File descriptor to write to
 * @param child_element_name Name of each list child element
 * @param value List element value
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 * @param print_value Pointer to a function that prints the list element
 */
void NixXML_print_list_element_xml(FILE *file, const char *child_element_name, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value);

/**
 * Prints an XML representation of a list.
 *
 * @param file File descriptor to write to
 * @param list A list-like data structure
 * @param child_element_name Name of each list child element
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 * @param print_list_elements Pointer to a function that iterates over members in the list-like data structure and prints their values
 * @param print_value Pointer to a function that prints the element value
 */
void NixXML_print_list_xml(FILE *file, const void *list, const char *child_element_name, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLListMembersFunc print_list_elements, NixXML_PrintXMLValueFunc print_value);

/**
 * Prints a simple XML representation of an attribute in which the key
 * corresponds to the attribute name and text sub element to the value.
 *
 * @param file File descriptor to write to
 * @param name Attribute name
 * @param value Attribute value
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 * @param print_value Pointer to a function that prints the attribute value
 */
void NixXML_print_simple_attribute_xml(FILE *file, const char *name, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value);

/**
 * Prints a verbose XML representation of an attribute in which the attribute
 * name is used as a property and a text sub element the value.
 *
 * @param file File descriptor to write to
 * @param child_element_name Name of each attribute child element
 * @param name_property_name Name of the name property
 * @param name Attribute name
 * @param value Attribute value
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 * @param print_value Pointer to a function that prints the attribute value
 */
void NixXML_print_verbose_attribute_xml(FILE *file, const char *child_element_name, const char *name_property_name, const char *name, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value);

/**
 * Prints a simple XML representation of an attribute set in which the key
 * corresponds to the attribute name and text sub element to the value.
 *
 * @param file File descriptor to write to
 * @param table A table-like data structure
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 * @param print_attributes Pointer to a function that iterates over the attributes and prints their assignments
 * @param print_value Pointer to a function that prints the attribute value
 */
void NixXML_print_simple_attrset_xml(FILE *file, const void *table, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintSimpleXMLMembersFunc print_attributes, NixXML_PrintXMLValueFunc print_value);

/**
 * Prints a verbose XML representation of an attribute set in which the key
 * corresponds to an element property and text sub element to the value.
 *
 * @param file File descriptor to write to
 * @param table A table-like data structure
 * @param child_element_name Name of each attribute child element
 * @param name_property_name Name of the name property
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 * @param print_attributes Pointer to a function that iterates over the attributes and prints their assignments
 * @param print_value Pointer to a function that prints the attribute value
 */
void NixXML_print_verbose_attrset_xml(FILE *file, const void *table, const char *child_element_name, const char *name_property_name, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintVerboseXMLMembersFunc print_attributes, NixXML_PrintXMLValueFunc print_value);

#ifdef __cplusplus
}
#endif

#endif
