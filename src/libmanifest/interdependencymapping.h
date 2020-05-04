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

/**
 * Compares two interdependency mappings and indicates its sort order
 *
 * @param l Pointer to an inter dependency mapping instance
 * @param r Pointer to an inter dependency mapping instance
 * @return 0 if they are equal, < 0 if l comes before r, > 0 if l comes after r
 */
gint compare_interdependency_mappings(const InterDependencyMapping **l, const InterDependencyMapping **r);

/**
 * Parses an attribute set member and inserts it into an InterDependencyMapping
 * struct in one go.
 *
 * @param table Inter dependency mapping instance
 * @param key Attribute key
 * @param value Inter-dependency mapping instance
 * @param userdata Arbitrary user data that is propagated to all parse functions
 */
void insert_interdependency_mapping_attributes(void *table, const xmlChar *key, void *value, void *userdata);

/**
 * Parses an XML document representing an interdependency mapping
 *
 * @param element XML element to parse.
 * @param userdata Arbitrary user data that is propagated to all parse functions
 * @return Pointer to a parsed interdependency mapping struct instance. It should be removed from memory with delete_interdependency_mapping()
 */
void *parse_interdependency_mapping(xmlNodePtr element, void *userdata);

/**
 * Deletes an interdependency mapping instance from heap memory.
 *
 * @param mapping Inter dependency mapping instance
 */
void delete_interdependency_mapping(InterDependencyMapping *mapping);

/**
 * Checks whether an interdependency mapping is valid.
 *
 * @param mapping Inter-dependency mapping instance
 * @return TRUE if the interdependency mapping is valid, else FALSE
 */
int check_interdependency_mapping(const InterDependencyMapping *mapping);

/**
 * Prints a Nix expression representation of an interdependency mapping
 *
 * @param file File descriptor to write to
 * @param mapping Inter-dependency mapping instance
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_interdependency_mapping_nix(FILE *file, const InterDependencyMapping *mapping, const int indent_level, void *userdata);

/**
 * Prints an XML representation of an interdependency mapping.
 *
 * @param file File descriptor to write to
 * @param mapping Inter-dependency mapping instance
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_interdependency_mapping_xml(FILE *file, const InterDependencyMapping *mapping, const int indent_level, const char *type_property_name, void *userdata);

#endif
