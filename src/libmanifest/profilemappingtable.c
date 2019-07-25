/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2018  Sander van der Burg
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

#include "profilemappingtable.h"
#include <nixxml-print-nix.h>
#include <nixxml-print-xml.h>
#include <nixxml-ghashtable.h>
#include "propertytable-util.h"

GHashTable *parse_profile_mapping_table(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_g_hash_table_verbose(element, "profile", "name", userdata, NixXML_parse_value);
}

void delete_profile_mapping_table(GHashTable *profile_mapping_table)
{
    delete_hash_table(profile_mapping_table, (DeleteFunction)g_free);
}

int check_profile_mapping_table(GHashTable *profile_mapping_table)
{
    return check_property_table(profile_mapping_table);
}

int compare_profile_mapping_tables(GHashTable *profile_mapping_table1, GHashTable *profile_mapping_table2)
{
    return compare_property_tables(profile_mapping_table1, profile_mapping_table2);
}

void print_profile_mapping_table_nix(FILE *file, GHashTable *profile_mapping_table, const int indent_level, void *userdata)
{
    NixXML_print_g_hash_table_nix(file, profile_mapping_table, indent_level, userdata, NixXML_print_store_path_nix);
}

void print_profile_mapping_table_xml(FILE *file, GHashTable *profile_mapping_table, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_g_hash_table_verbose_xml(file, profile_mapping_table, "profile", "name", indent_level, NULL, userdata, NixXML_print_string_xml);
}
