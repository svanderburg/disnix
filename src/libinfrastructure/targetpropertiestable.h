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

#ifndef __DISNIX_TARGETPROPERTIESTABLE_H
#define __DISNIX_TARGETPROPERTIESTABLE_H
#include <libxml/parser.h>
#include <glib.h>
#include <nixxml-types.h>

/**
 * Parses an XML document representing a target properties table
 *
 * @param element XML element to parse.
 * @param userdata Arbitrary user data that is propagated to all parse functions
 * @return Pointer to a parsed containers table. It should be removed from memory with delete_target_properties_table()
 */
void *parse_target_properties_table(xmlNodePtr element, void *userdata);

/**
 * Deletes a target properties table and its content from heap memory.
 *
 * @param target_properties_table A target properties table
 */
void delete_target_properties_table(GHashTable *target_properties_table);

/**
 * Checks whether two target property tables have the same content.
 *
 * @param left A target properties table
 * @param right A target properties table
 * @return TRUE if they are equal, else FALSE
 */
NixXML_bool compare_target_properties_tables(GHashTable *left, GHashTable *right);

/**
 * Prints a Nix expression representation of a target properties table.
 *
 * @param file File descriptor to write to
 * @param target_properties_table A target properties table
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_target_properties_table_nix(FILE *file, GHashTable *target_properties_table, const int indent_level, void *userdata);

/**
 * Prints an XML representation of a target properties table.
 *
 * @param file File descriptor to write to
 * @param target_properties_table A target properties table
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_target_properties_table_xml(FILE *file, GHashTable *target_properties_table, const int indent_level, const char *type_property_name, void *userdata);

#endif
