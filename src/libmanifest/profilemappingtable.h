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

#ifndef __DISNIX_PROFILEMAPPINGTABLE_H
#define __DISNIX_PROFILEMAPPINGTABLE_H
#include <glib.h>
#include <libxml/parser.h>
#include <targetstable.h>

/**
 * Creates a new array with distribution items from the corresponding sub
 * section in an XML document.
 *
 * @param element XML root element of the sub section defining the mappings
 * @return GHashTable mapping targets to Nix profiles
 */
GHashTable *parse_profile_mapping_table(xmlNodePtr element, void *userdata);

/**
 * Deletes an array with distribution items.
 *
 * @param profile_mapping_table GHashTable mapping targets to Nix profiles
 */
void delete_profile_mapping_table(GHashTable *profile_mapping_table);

int check_profile_mapping_table(GHashTable *profile_mapping_table);

int compare_profile_mapping_tables(GHashTable *profile_mapping_table1, GHashTable *profile_mapping_table2);

void print_profile_mapping_table_nix(FILE *file, const void *value, const int indent_level, void *userdata);

#endif
