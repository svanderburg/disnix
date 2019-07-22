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

#include "nixxml-glib.h"
#include "nixxml-parse-generic.h"
#include "nixxml-print-generic-nix.h"
#include "nixxml-print-generic-xml.h"
#include "nixxml-generate-env-generic.h"
#include "nixxml-ghashtable.h"
#include "nixxml-gptrarray.h"

void *NixXML_generic_parse_expr_glib(xmlNodePtr element, const char *type_property_name, const char *name_property_name, void *userdata)
{
    return NixXML_generic_parse_expr(element, type_property_name, name_property_name, NixXML_create_g_ptr_array, NixXML_create_g_hash_table, NixXML_add_value_to_g_ptr_array, NixXML_insert_into_g_hash_table, NixXML_finalize_g_ptr_array);
}

void NixXML_print_generic_expr_glib_nix(FILE *file, const NixXML_Node *node, const int indent_level)
{
    NixXML_print_generic_expr_nix(file, node, indent_level, NixXML_print_g_ptr_array_elements_nix, NixXML_print_g_hash_table_attributes_nix);
}

void NixXML_print_generic_expr_glib_ordered_nix(FILE *file, const NixXML_Node *node, const int indent_level)
{
    NixXML_print_generic_expr_nix(file, node, indent_level, NixXML_print_g_ptr_array_elements_nix, NixXML_print_g_hash_table_ordered_attributes_nix);
}

void NixXML_print_generic_expr_glib_simple_xml(FILE *file, const NixXML_Node *node, const int indent_level, const char *root_element_name, const char *list_element_name, const char *type_property_name)
{
    NixXML_print_generic_expr_simple_xml(file, node, indent_level, root_element_name, list_element_name, type_property_name, NixXML_print_g_ptr_array_elements_xml, NixXML_print_g_hash_table_simple_attributes_xml);
}

void NixXML_print_generic_expr_glib_simple_ordered_xml(FILE *file, const NixXML_Node *node, const int indent_level, const char *root_element_name, const char *list_element_name, const char *type_property_name)
{
    NixXML_print_generic_expr_simple_xml(file, node, indent_level, root_element_name, list_element_name, type_property_name, NixXML_print_g_ptr_array_elements_xml, NixXML_print_g_hash_table_simple_ordered_attributes_xml);
}

void NixXML_print_generic_expr_glib_verbose_xml(FILE *file, const NixXML_Node *node, const int indent_level, const char *root_element_name, const char *attr_element_name, const char *name_property_name, const char *list_element_name, const char *type_property_name)
{
    NixXML_print_generic_expr_verbose_xml(file, node, indent_level, root_element_name, list_element_name, attr_element_name, name_property_name, type_property_name, NixXML_print_g_ptr_array_elements_xml, NixXML_print_g_hash_table_verbose_attributes_xml);
}

void NixXML_print_generic_expr_glib_verbose_ordered_xml(FILE *file, const NixXML_Node *node, const int indent_level, const char *root_element_name, const char *attr_element_name, const char *name_property_name, const char *list_element_name, const char *type_property_name)
{
    NixXML_print_generic_expr_verbose_xml(file, node, indent_level, root_element_name, list_element_name, attr_element_name, name_property_name, type_property_name, NixXML_print_g_ptr_array_elements_xml, NixXML_print_g_hash_table_verbose_ordered_attributes_xml);
}

xmlChar **NixXML_generate_env_vars_generic_glib(GHashTable *hash_table)
{
    NixXML_GenerateEnvGenericParams params;
    params.generate_list_env = NixXML_generate_env_value_from_g_ptrarray;
    return NixXML_generate_env_vars_from_g_hash_table(hash_table, &params, NixXML_generate_env_value_from_node);
}
