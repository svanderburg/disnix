/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2021  Sander van der Burg
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

#include "targetpropertiestable.h"
#include <nixxml-ghashtable.h>
#include <nixxml-glib.h>

static void *generic_parse_expr(xmlNodePtr element, void *userdata)
{
    return NixXML_generic_parse_verbose_expr_glib(element, "type", "name", userdata);
}

void *parse_target_properties_table(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_g_hash_table_verbose(element, "property", "name", userdata, generic_parse_expr);
}

void delete_target_properties_table(GHashTable *target_properties_table)
{
    NixXML_delete_g_hash_table(target_properties_table, (NixXML_DeleteGHashTableValueFunc)NixXML_delete_node_glib);
}

NixXML_bool compare_target_properties_tables(GHashTable *left, GHashTable *right)
{
    return NixXML_compare_g_hash_tables(left, right, (NixXML_CompareGHashTableValueFunc)NixXML_compare_nodes_glib);
}

static void print_generic_expr_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    NixXML_print_generic_expr_glib_nix(file, (const NixXML_Node*)value, indent_level);
}

void print_target_properties_table_nix(FILE *file, GHashTable *target_properties_table, const int indent_level, void *userdata)
{
    NixXML_print_g_hash_table_nix(file, target_properties_table, indent_level, userdata, print_generic_expr_nix);
}

static void print_generic_expr_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_generic_expr_glib_verbose_xml(file, (const NixXML_Node*)value, indent_level, "property", "attr", "name", "list", "type");
}

void print_target_properties_table_xml(FILE *file, GHashTable *target_properties_table, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_g_hash_table_verbose_xml(file, target_properties_table, "property", "name", indent_level, NULL, userdata, print_generic_expr_xml);
}
