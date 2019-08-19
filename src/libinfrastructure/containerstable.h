/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2019  Sander van der Burg
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

#ifndef __DISNIX_CONTAINERSTABLE_H
#define __DISNIX_CONTAINERSTABLE_H
#include <libxml/parser.h>
#include <glib.h>

/**
 * Parses an XML document representing a containers table
 *
 * @param element XML element to parse.
 * @param userdata Arbitrary user data that is propagated to all parse functions
 * @return Pointer to a parsed containers table. It should be removed from memory with delete_containers_table()
 */
void *parse_containers_table(xmlNodePtr element, void *userdata);

/**
 * Deletes a containers table and its content from heap memory.
 *
 * @param containers_table A containers table
 */
void delete_containers_table(GHashTable *containers_table);

/**
 * Checks whether two container tables have the same content.
 *
 * @param containers_table1 A containers table
 * @param containers_table2 A containers table
 * @return TRUE if they are equal, else FALSE
 */
int compare_container_tables(GHashTable *containers_table1, GHashTable *containers_table2);

/**
 * Prints a Nix expression representation of a containers table.
 *
 * @param file File descriptor to write to
 * @param containers_table A containers table
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_containers_table_nix(FILE *file, GHashTable *containers_table, const int indent_level, void *userdata);

/**
 * Prints an XML representation of a containers table.
 *
 * @param file File descriptor to write to
 * @param containers_table A containers table
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_containers_table_xml(FILE *file, GHashTable *containers_table, const int indent_level, const char *type_property_name, void *userdata);

#endif
