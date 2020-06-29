/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2020  Sander van der Burg
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

#ifndef __DISNIX_MANIFESTSERVICESTABLE_H
#define __DISNIX_MANIFESTSERVICESTABLE_H
#include <stdio.h>
#include <glib.h>
#include <libxml/parser.h>
#include "manifestservice.h"

/**
 * Parses an XML document representing a manifest service table
 *
 * @param element XML element to parse.
 * @param userdata Arbitrary user data that is propagated to all parse functions
 * @return Pointer to a parsed services table. It should be removed from memory with delete_services_table()
 */
GHashTable *parse_services_table(xmlNodePtr element, void *userdata);

/**
 * Deletes a services table and its content from heap memory.
 *
 * @param services_table A services table
 */
void delete_services_table(GHashTable *services_table);

/**
 * Checks a services table and its content for validity.
 *
 * @param services_table A services table
 * @return TRUE if the services table is valid, else FALSE
 */
NixXML_bool check_services_table(GHashTable *services_table);

/**
 * Checks whether two services tables have the same content.
 *
 * @param services_table1 A services table
 * @param services_table2 A services table
 * @return TRUE if they are equal, else FALSE
 */
NixXML_bool compare_services_tables(GHashTable *services_table1, GHashTable *services_table2);

/**
 * Merges the content of two services table into one services table without
 * copying the values. The resulting hash table should be removed with
 * g_hash_table_destroy() if it is not needed anymore.
 *
 * @param left A services table
 * @param right A services table
 * @return A new hash table with the content of both tables merged
 */
GHashTable *generate_union_services_table(GHashTable *left, GHashTable *right);

/**
 * Prints a Nix expression representation of a services table.
 *
 * @param file File descriptor to write to
 * @param services_table A services table
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_services_table_nix(FILE *file, GHashTable *services_table, const int indent_level, void *userdata);

/**
 * Prints an XML representation of a services table.
 *
 * @param file File descriptor to write to
 * @param services_table A services table
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_services_table_xml(FILE *file, GHashTable *services_table, const int indent_level, const char *type_property_name, void *userdata);

#endif
