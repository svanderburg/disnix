/*
 * Copyright (c) 2019-2022 Sander van der Burg
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

#include "nixxml-print-generic-xml.h"

void NixXML_print_expr_simple_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_Node *node = (NixXML_Node*)value;
    NixXML_SimplePrintExprParams *params = (NixXML_SimplePrintExprParams*)userdata;

    switch(node->type)
    {
        case NIX_XML_TYPE_STRING:
            NixXML_print_string_xml(file, node->value, indent_level, type_property_name, userdata);
            break;
        case NIX_XML_TYPE_PATH:
            NixXML_print_string_as_path_xml(file, node->value, indent_level, type_property_name, userdata);
            break;
        case NIX_XML_TYPE_INT:
            NixXML_print_string_as_int_xml(file, node->value, indent_level, type_property_name, userdata);
            break;
        case NIX_XML_TYPE_FLOAT:
            NixXML_print_string_as_float_xml(file, node->value, indent_level, type_property_name, userdata);
            break;
        case NIX_XML_TYPE_BOOL:
            NixXML_print_string_as_bool_xml(file, node->value, indent_level, type_property_name, userdata);
            break;
        case NIX_XML_TYPE_LIST:
            NixXML_print_list_xml(file, node->value, params->list_element_name, indent_level, type_property_name, userdata, params->print_list_elements, NixXML_print_expr_simple_xml);
            break;
        case NIX_XML_TYPE_ATTRSET:
            NixXML_print_simple_attrset_xml(file, node->value, indent_level, type_property_name, userdata, params->print_attributes, NixXML_print_expr_simple_xml);
            break;
    }
}

void NixXML_print_generic_expr_simple_xml(FILE *file, const NixXML_Node *value, const int indent_level, const char *root_element_name, const char *list_element_name, const char *type_property_name, NixXML_PrintXMLListMembersFunc print_list_elements, NixXML_PrintSimpleXMLMembersFunc print_attributes)
{
    NixXML_SimplePrintExprParams params;
    params.list_element_name = list_element_name;
    params.print_list_elements = print_list_elements;
    params.print_attributes = print_attributes;

    NixXML_print_open_root_tag(file, root_element_name);
    NixXML_print_expr_simple_xml(file, value, indent_level, type_property_name, &params);
    NixXML_print_close_root_tag(file, list_element_name);
}

void NixXML_print_expr_verbose_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_Node *node = (NixXML_Node*)value;
    NixXML_VerbosePrintExprParams *params = (NixXML_VerbosePrintExprParams*)userdata;

    switch(node->type)
    {
        case NIX_XML_TYPE_INT:
            NixXML_print_string_as_int_xml(file, node->value, indent_level, type_property_name, userdata);
            break;
        case NIX_XML_TYPE_STRING:
            NixXML_print_string_xml(file, node->value, indent_level, type_property_name, userdata);
            break;
        case NIX_XML_TYPE_PATH:
            NixXML_print_string_as_path_xml(file, node->value, indent_level, type_property_name, userdata);
            break;
        case NIX_XML_TYPE_FLOAT:
            NixXML_print_string_as_float_xml(file, node->value, indent_level, type_property_name, userdata);
            break;
        case NIX_XML_TYPE_BOOL:
            NixXML_print_string_as_bool_xml(file, node->value, indent_level, type_property_name, userdata);
            break;
        case NIX_XML_TYPE_LIST:
            NixXML_print_list_xml(file, node->value, params->list_element_name, indent_level, type_property_name, userdata, params->print_list_elements, NixXML_print_expr_verbose_xml);
            break;
        case NIX_XML_TYPE_ATTRSET:
            NixXML_print_verbose_attrset_xml(file, node->value, params->attr_element_name, params->name_property_name, indent_level, type_property_name, userdata, params->print_attributes, NixXML_print_expr_verbose_xml);
            break;
    }
}

void NixXML_print_generic_expr_verbose_xml(FILE *file, const NixXML_Node *value, const int indent_level, const char *root_element_name, const char *list_element_name, const char *attr_element_name, const char *name_property_name, const char *type_property_name, NixXML_PrintXMLListMembersFunc print_list_elements, NixXML_PrintVerboseXMLMembersFunc print_attributes)
{
    NixXML_VerbosePrintExprParams params;
    params.list_element_name = list_element_name;
    params.attr_element_name = attr_element_name;
    params.name_property_name = name_property_name;
    params.print_list_elements = print_list_elements;
    params.print_attributes = print_attributes;

    NixXML_print_open_root_tag(file, root_element_name);
    NixXML_print_expr_verbose_xml(file, value, indent_level, type_property_name, &params);
    NixXML_print_close_root_tag(file, root_element_name);
}
