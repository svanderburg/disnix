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

#ifndef __DISNIX_INTERDEPENDENCY_MAPPING_H
#define __DISNIX_INTERDEPENDENCY_MAPPING_H

#include <glib.h>
#include <libxml/parser.h>

/**
 * @brief Contains the values that constitute a key uniquely referring to a service that needs to be activated.
 */
typedef struct
{
    /** Hash code that uniquely defines a service */
    xmlChar *service;
    /** Name of the container to which the service is deployed */
    xmlChar *container;
    /** Name of the target machine to which the service is deployed */
    xmlChar *target;
}
InterDependencyMapping;

gint compare_interdependency_mappings(const InterDependencyMapping **l, const InterDependencyMapping **r);

void insert_interdependency_mapping_attributes(void *table, const xmlChar *key, void *value, void *userdata);

void *parse_interdependency_mapping(xmlNodePtr element, void *userdata);

int check_interdependency_mapping(const InterDependencyMapping *mapping);

void delete_interdependency_mapping(InterDependencyMapping *mapping);

/**
 * Returns the dependency with the given keys in the dependsOn array.
 *
 * @param depends_on dependsOn array
 * @param key Key of the dependency to find
 * @return The dependency mapping with the specified keys, or NULL if it cannot be found
 */
InterDependencyMapping *find_interdependency_mapping(const GPtrArray *interdependency_mapping_array, const InterDependencyMapping *key);

void print_interdependency_mapping_nix(FILE *file, const void *value, const int indent_level, void *userdata);

void print_interdependency_mapping_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata);

#endif
