/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2022  Sander van der Burg
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

#ifndef __DISNIX_PROFILEMAPPINGTABLE_H
#define __DISNIX_PROFILEMAPPINGTABLE_H
#include <glib.h>
#include <libxml/parser.h>
#include <nixxml-types.h>
#include <targetstable.h>

/**
 * Creates a new hash table with profile mappings from the corresponding sub
 * section in an XML document.
 *
 * @param element XML root element of the sub section defining the mappings
 * @param userdata Arbitrary user data that is propagated to all parse functions
 * @return GHashTable mapping targets to Nix profiles
 */
GHashTable *parse_profile_mapping_table(xmlNodePtr element, void *userdata);

/**
 * Deletes a profile mapping table from heap memory
 *
 * @param profile_mapping_table GHashTable mapping targets to Nix profiles
 */
void delete_profile_mapping_table(GHashTable *profile_mapping_table);

/**
 * Checks whether the contents of a profile mapping table is valid.
 *
 * @param profile_mapping_table GHashTable mapping targets to Nix profiles
 * @return TRUE if the profile mapping tables are valid, else FALSE
 */
NixXML_bool check_profile_mapping_table(GHashTable *profile_mapping_table);

/**
 * Checks whether two profile mapping tables have the same content.
 *
 * @param profile_mapping_table1 GHashTable mapping targets to Nix profiles
 * @param profile_mapping_table2 GHashTable mapping targets to Nix profiles
 * @return TRUE if the profile mapping tables are equal, else FALSE
 */
NixXML_bool compare_profile_mapping_tables(GHashTable *profile_mapping_table1, GHashTable *profile_mapping_table2);

/**
 * Prints a Nix expression representation of a profile mapping table.
 *
 * @param file File descriptor to write to
 * @param profile_mapping_table GHashTable mapping targets to Nix profiles
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_profile_mapping_table_nix(FILE *file, GHashTable *profile_mapping_table, const int indent_level, void *userdata);

/**
 * Prints an XML representation of a profile mapping table.
 *
 * @param file File descriptor to write to
 * @param profile_mapping_table GHashTable mapping targets to Nix profiles
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_profile_mapping_table_xml(FILE *file, GHashTable *profile_mapping_table, const int indent_level, const char *type_property_name, void *userdata);

#endif
