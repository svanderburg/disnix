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

#ifndef __NIXXML_PRINT_NIX_H
#define __NIXXML_PRINT_NIX_H
#include <stdio.h>
#include "nixxml-print.h"

/**
 * Prints a Nix representation of a value.
 *
 * @param file File descriptor to write to
 * @param value Pointer to a data structure to print
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
typedef void (*NixXML_PrintValueFunc) (FILE *file, const void *value, const int indent_level, void *userdata);

/**
 * Prints a Nix representation of the members of a list or attribute set.
 *
 * @param file File descriptor to write to
 * @param value Pointer to a data structure to print
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 * @param print_value Pointer to a function that prints an attribute value
 */
typedef void (*NixXML_PrintMembersFunc) (FILE *file, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Prints a string, quotes it and escapes it, if required.
 *
 * @param file File descriptor to write to
 * @param value Pointer to the first character of a string
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void NixXML_print_string_nix(FILE *file, const void *value, const int indent_level, void *userdata);

/**
 * Prints a string as a value.
 *
 * @param file File descriptor to write to
 * @param value Pointer to the first character of a string
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void NixXML_print_value_nix(FILE *file, const void *value, const int indent_level, void *userdata);

/**
 * Prints a string as a path, escaped it when needed.
 *
 * @param file File descriptor to write to
 * @param value Pointer to the first character of a string
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void NixXML_print_path_nix(FILE *file, const void *value, const int indent_level, void *userdata);

/**
 * Prints an interger.
 *
 * @param file File descriptor to write to
 * @param value Pointer to an integer
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void NixXML_print_int_nix(FILE *file, const void *value, const int indent_level, void *userdata);

/**
 * Prints a reference to a store path.
 *
 * @param file File descriptor to write to
 * @param value Pointer to a string representing a path in the Nix store
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void NixXML_print_store_path_nix(FILE *file, const void *value, const int indent_level, void *userdata);

/**
 * Prints a Nix expression representation of a list element.
 *
 * @param file File descriptor to write to
 * @param value List element value
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 * @param print_value Pointer to a function that prints an element value
 */
void NixXML_print_list_element_nix(FILE *file, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value);

/**
 * Prints a Nix expression representation of a list.
 *
 * @param file File descriptor to write to
 * @param list A list-like data structure
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 * @param print_list_elements Pointer to a function that prints the list elements
 * @param print_value Pointer to a function that prints an element value
 */
void NixXML_print_list_nix(FILE *file, const void *list, const int indent_level, void *userdata, NixXML_PrintMembersFunc print_list_elements, NixXML_PrintValueFunc print_value);

/**
 * Prints a Nix expression representation of an attribute.
 *
 * @param file File descriptor to write to
 * @param name Attribute name
 * @param value Attribute value
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 * @param print_value Pointer to a function that prints an attribute value
 */
void NixXML_print_attribute_nix(FILE *file, const char *name, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value);

/**
 * Prints a Nix expression representation of an attribute set.
 *
 * @param file File descriptor to write to
 * @param table A table-like data structure
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 * @param print_attributes Pointer to a function that prints the attributes
 * @param print_value Pointer to a function that prints an attribute value
 */
void NixXML_print_attrset_nix(FILE *file, const void *table, const int indent_level, void *userdata, NixXML_PrintMembersFunc print_attributes, NixXML_PrintValueFunc print_value);

#ifdef __cplusplus
}
#endif

#endif
