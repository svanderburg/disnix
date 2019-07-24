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

#ifndef __DISNIX_TARGETSTABLE_H
#define __DISNIX_TARGETSTABLE_H
#include <libxml/parser.h>
#include <glib.h>
#include "target.h"

/**
 * Parses all targets from a sub section in an XML document.
 *
 * @param element Root XML element that defines targets
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
 * @return GHashTable with targets
 */
GHashTable *create_targets_table_from_nix(char *infrastructure_expr, char *default_target_property, char *default_client_interface);

/**
 * Creates a hash table with targets from an infrastructure XML configuration
 *
 * @param infrastructure_xml Path to the infrastructure XML file
 * @return GHashTable with targets
 */
GHashTable *create_targets_table_from_xml(const char *infrastructure_xml);

/**
 * Creates a hash table with targets from an infrastructure configuration
 *
 * @param infrastructure_expr Path to the infrastructure configure file
 * @param xml TRUE to indicate the input is in XML, FALSE that it is a Nix expression
 * @return GHashTable with targets
 */
GHashTable *create_targets_table(gchar *infrastructure_expr, const int xml, char *default_target_property, char *default_client_interface);

/**
 * Deletes a hash table with targets
 *
 * @param targets_table Targets table to delete
 */
void delete_targets_table(GHashTable *targets_table);

int check_targets_table(GHashTable *targets_table);

int compare_targets_tables(GHashTable *targets_table1, GHashTable *targets_table2);

void print_targets_table_nix(FILE *file, GHashTable *targets_table, const int indent_level, void *userdata);

void print_targets_table_xml(FILE *file, GHashTable *targets_table, const int indent_level, const char *type_property_name, void *userdata);

#endif
