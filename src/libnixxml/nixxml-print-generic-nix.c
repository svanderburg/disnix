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

#include "nixxml-print-generic-nix.h"

void NixXML_print_expr_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    NixXML_Node *node = (NixXML_Node*)value;
    NixXML_PrintExprParams *params = (NixXML_PrintExprParams*)userdata;

    switch(node->type)
    {
        case NIX_XML_TYPE_STRING:
            NixXML_print_string_nix(file, node->value, indent_level, userdata);
            break;
        case NIX_XML_TYPE_INT:
        case NIX_XML_TYPE_FLOAT:
        case NIX_XML_TYPE_BOOL:
            NixXML_print_value_nix(file, node->value, indent_level, userdata);
            break;
        case NIX_XML_TYPE_LIST:
            NixXML_print_list_nix(file, node->value, indent_level, userdata, params->print_list_elements, NixXML_print_expr_nix);
            break;
        case NIX_XML_TYPE_ATTRSET:
            NixXML_print_attrset_nix(file, node->value, indent_level, userdata, params->print_attributes, NixXML_print_expr_nix);
            break;
    }
}

void NixXML_print_generic_expr_nix(FILE *file, const NixXML_Node *value, const int indent_level, NixXML_PrintMembersFunc print_list_elements, NixXML_PrintMembersFunc print_attributes)
{
    NixXML_PrintExprParams params;
    params.print_list_elements = print_list_elements;
    params.print_attributes = print_attributes;
    NixXML_print_expr_nix(file, value, indent_level, &params);
}
