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

#ifndef __DISNIX_INTERDEPENDENCY_MAPPING_ARRAY_H
#define __DISNIX_INTERDEPENDENCY_MAPPING_ARRAY_H

#include <glib.h>
#include <libxml/parser.h>
#include "interdependencymapping.h"

GPtrArray *parse_interdependency_mapping_array(xmlNodePtr element, void *userdata);

int check_interdependency_mapping_array(const GPtrArray *interdependency_mapping_array);

void delete_interdependency_mapping_array(GPtrArray *interdependency_mapping_array);

int compare_interdependency_mapping_arrays(const GPtrArray *interdependency_mapping_array1, const GPtrArray *interdependency_mapping_array2);

void print_interdependency_mapping_array_nix(FILE *file, const GPtrArray *interdependency_mapping_array, const int indent_level, void *userdata);

void print_interdependency_mapping_array_xml(FILE *file, const GPtrArray *interdependency_mapping_array, const int indent_level, const char *type_property_name, void *userdata);

#endif
