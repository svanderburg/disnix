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

/**
 * Creates an array with interdependency mappings from the corresponding sub
 * section in an XML document. The resulting array must be freed with
 * delete_interdependency_mapping_array() when it is not need anymore.
 *
 * @param element XML root element of the sub section defining the mappings
 * @param userdata Arbitrary user data that is propagated to all parse functions
 * @return GPtrArray containing interdependency mappings
 */
GPtrArray *parse_interdependency_mapping_array(xmlNodePtr element, void *userdata);

/**
 * Deletes an inter dependency mapping array and its content from heap memory.
 *
 * @param interdependency_mapping_array A GPtrArray with interdependency mappings
 */
void delete_interdependency_mapping_array(GPtrArray *interdependency_mapping_array);

/**
 * Checks whether an array of inter dependency mappings is valid.
 *
 * @param interdependency_mapping_array A GPtrArray with interdependency mappings
 * @return TRUE when the content is valid, else FALSE
 */
int check_interdependency_mapping_array(const GPtrArray *interdependency_mapping_array);

/**
 * Compares two interdependency mapping arrays and checks whether they have the
 * same content.
 *
 * @param interdependency_mapping_array1 A GPtrArray with interdependency mappings
 * @param interdependency_mapping_array2 A GPtrArray with interdependency mappings
 * @return TRUE if their content is equal, else FALSE
 */
int compare_interdependency_mapping_arrays(const GPtrArray *interdependency_mapping_array1, const GPtrArray *interdependency_mapping_array2);

/**
 * Prints a Nix expression representation of an inter-dependency mapping array.
 *
 * @param file File descriptor to write to
 * @param interdependency_mapping_array A GPtrArray with interdependency mappings
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_interdependency_mapping_array_nix(FILE *file, const GPtrArray *interdependency_mapping_array, const int indent_level, void *userdata);

/**
 * Prints an XML representation of an inter-dependency mapping array.
 *
 * @param file File descriptor to write to
 * @param interdependency_mapping_array A GPtrArray with interdependency mappings
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_interdependency_mapping_array_xml(FILE *file, const GPtrArray *interdependency_mapping_array, const int indent_level, const char *type_property_name, void *userdata);

/**
 * Returns the dependency with the given keys in the inter dependency mapping array.
 *
 * @param interdependency_mapping_array An array of inter dependency mappings
 * @param key Key of the dependency to find
 * @return The dependency mapping with the specified keys, or NULL if it cannot be found
 */
InterDependencyMapping *find_interdependency_mapping(const GPtrArray *interdependency_mapping_array, const InterDependencyMapping *key);

#endif
