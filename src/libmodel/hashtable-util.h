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

#ifndef __DISNIX_HASHTABLE_UTIL_H
#define __DISNIX_HASHTABLE_UTIL_H
#include <glib.h>
#include <libxml/parser.h>

typedef int (*CheckFunction) (const gpointer value);

typedef int (*DeleteFunction) (gpointer value);

typedef int (*CompareFunction) (const gpointer left, const gpointer right);

void delete_hash_table(GHashTable *hash_table, DeleteFunction delete_function);

void delete_property_table(GHashTable *property_table);

int compare_hash_tables(GHashTable *hash_table1, GHashTable *hash_table2, CompareFunction compare_function);

int compare_xml_strings(const xmlChar *left, const xmlChar *right);

int compare_property_tables(GHashTable *property_table1, GHashTable *property_table2);

int check_value_is_not_null(const gpointer value);

int check_hash_table(GHashTable *hash_table, CheckFunction check_function);

int check_property_table(GHashTable *property_table);

#endif