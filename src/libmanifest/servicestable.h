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

#ifndef __DISNIX_SERVICESTABLE_H
#define __DISNIX_SERVICESTABLE_H
#include <stdio.h>
#include <glib.h>
#include <libxml/parser.h>
#include "manifestservice.h"

GHashTable *parse_services_table(xmlNodePtr element, void *userdata);

void delete_services_table(GHashTable *services_table);

int check_services_table(GHashTable *services_table);

int compare_services_tables(GHashTable *services_table1, GHashTable *services_table2);

GHashTable *generate_union_services_table(GHashTable *left, GHashTable *right);

void print_services_table_nix(FILE *file, const void *value, const int indent_level, void *userdata);

void print_services_table_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata);

#endif
