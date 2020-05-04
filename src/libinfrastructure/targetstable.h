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

#ifndef __DISNIX_TARGETSTABLE_H
#define __DISNIX_TARGETSTABLE_H
#include <libxml/parser.h>
#include <glib.h>
#include "target.h"

/**
 * Parses all targets from a sub section in an XML document.
 *
 * @param element Root XML element that defines targets
 * @param userdata Arbitrary user data that is propagated to all parse functions
 * @return GHashTable of targets
 */
GHashTable *parse_targets_table(xmlNodePtr element, void *userdata);

/**
 * Creates a hash table with targets from an XML document
 *
 * @param doc XML document composed by the XML parser
 * @return GHashTable with targets
 */
GHashTable *create_targets_table_from_doc(xmlDocPtr doc);

/**
 * Creates a hash table with targets from an infrastructure Nix expression
 *
 * @param infrastructure_expr Path to the infrastructure Nix expression
 * @param default_target_property Specifies the default target property to use if none was provided by a target
 * @param default_client_interface Specifies the default client interface to use if none was provided by a target
 * @return GHashTable with targets
 */
GHashTable *create_targets_table_from_nix(char *infrastructure_expr, char *default_target_property, char *default_client_interface);

/**
 * Creates a hash table with targets from an infrastructure XML configuration
 *
 * @param infrastructure_xml Path to the infrastructure XML file
 * @param default_target_property Specifies the default target property to use if none was provided by a target
 * @param default_client_interface Specifies the default client interface to use if none was provided by a target
 * @return GHashTable with targets
 */
GHashTable *create_targets_table_from_xml(const char *infrastructure_xml, char *default_target_property, char *default_client_interface);

/**
 * Creates a hash table with targets from an infrastructure configuration
 *
 * @param infrastructure_expr Path to the infrastructure configure file
 * @param xml TRUE to indicate the input is in XML, FALSE that it is a Nix expression
 * @param default_target_property Specifies the default target property to use if none was provided by a target
 * @param default_client_interface Specifies the default client interface to use if none was provided by a target
 * @return GHashTable with targets
 */
GHashTable *create_targets_table(gchar *infrastructure_expr, const int xml, char *default_target_property, char *default_client_interface);

/**
 * Deletes a hash table with targets from heap memory
 *
 * @param targets_table Targets table to delete
 */
void delete_targets_table(GHashTable *targets_table);

/**
 * Checks whether the content of the targets table is valid.
 *
 * @param targets_table GHashTable with targets
 * @return TRUE if all properties are valid, else FALSE
 */
int check_targets_table(GHashTable *targets_table);

/**
 * Compares the content of two target tables.
 *
 * @param targets_table1 GHashTable with targets
 * @param targets_table2 GHashTable with targets
 * @return TRUE if all the content is valid, else FALSE
 */
int compare_targets_tables(GHashTable *targets_table1, GHashTable *targets_table2);

/**
 * Prints a Nix expression representation of a targets table.
 *
 * @param file File descriptor to write to
 * @param targets_table GHashTable with targets
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_targets_table_nix(FILE *file, GHashTable *targets_table, const int indent_level, void *userdata);

/**
 * Prints an XML representation of a targets table.
 *
 * @param file File descriptor to write to
 * @param targets_table GHashTable with targets
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_targets_table_xml(FILE *file, GHashTable *targets_table, const int indent_level, const char *type_property_name, void *userdata);

#endif
