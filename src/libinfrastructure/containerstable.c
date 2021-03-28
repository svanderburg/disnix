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

#include "containerstable.h"
#include <nixxml-ghashtable.h>
#include "targetpropertiestable.h"

void *parse_containers_table(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_g_hash_table_verbose(element, "container", "name", userdata, parse_target_properties_table);
}

void delete_containers_table(GHashTable *containers_table)
{
    NixXML_delete_g_hash_table(containers_table, (NixXML_DeleteGHashTableValueFunc)delete_target_properties_table);
}

NixXML_bool compare_container_tables(GHashTable *containers_table1, GHashTable *containers_table2)
{
    return NixXML_compare_g_hash_tables(containers_table1, containers_table2, (NixXML_CompareGHashTableValueFunc)compare_target_properties_tables);
}

void print_containers_table_nix(FILE *file, GHashTable *containers_table, const int indent_level, void *userdata)
{
    NixXML_print_g_hash_table_nix(file, containers_table, indent_level, userdata, (NixXML_PrintValueFunc)print_target_properties_table_nix);
}

void print_containers_table_xml(FILE *file, GHashTable *containers_table, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_g_hash_table_verbose_xml(file, containers_table, "container", "name", indent_level, NULL, userdata, (NixXML_PrintXMLValueFunc)print_target_properties_table_xml);
}
