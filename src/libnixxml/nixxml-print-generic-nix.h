/*
 * Copyright (c) 2019-2020 Sander van der Burg
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

#ifndef __NIXXML_PRINT_GENERIC_NIX_H
#define __NIXXML_PRINT_GENERIC_NIX_H
#include <stdio.h>
#include "nixxml-node.h"
#include "nixxml-print-nix.h"

/**
 * @brief Generic parameters for printing Nix expressions that get propagated to all print functions
 */
typedef struct
{
    /** Pointer to a function that prints the list elements */
    NixXML_PrintMembersFunc print_list_elements;
    /** Pointer to a function that prints the attributes */
    NixXML_PrintMembersFunc print_attributes;
}
NixXML_PrintExprParams;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Internal function (that can be integrated into the printing infrastructure)
 * that generically prints a Nix representation of a data structure of XML_Node
 * objects.
 *
 * @param file File descriptor to write to
 * @param value Root XML_Node of the data structure to print
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata A NixXML_PrintExprParams object that gets propagated to all print functions
 */
void NixXML_print_expr_nix(FILE *file, const void *value, const int indent_level, void *userdata);

/**
 * Generically prints a Nix representation of a data structure of XML_Node
 * objects.
 *
 * @param file File descriptor to write to
 * @param value Root XML_Node of the data structure to print
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param print_list_elements Pointer to a function that prints the list elements
 * @param print_attributes Pointer to a function that prints the attributes
 */
void NixXML_print_generic_expr_nix(FILE *file, const NixXML_Node *value, const int indent_level, NixXML_PrintMembersFunc print_list_elements, NixXML_PrintMembersFunc print_attributes);

#ifdef __cplusplus
}
#endif

#endif
