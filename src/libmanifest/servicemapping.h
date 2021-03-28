/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2021  Sander van der Burg
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

#ifndef __DISNIX_SERVICEMAPPING_H
#define __DISNIX_SERVICEMAPPING_H
#include <glib.h>
#include <libxml/parser.h>
#include <nixxml-types.h>

/**
 * @brief Enumeration of possible states for a service mapping.
 */
typedef enum
{
    SERVICE_MAPPING_DEACTIVATED,
    SERVICE_MAPPING_IN_PROGRESS,
    SERVICE_MAPPING_ACTIVATED,
    SERVICE_MAPPING_ERROR
}
ServiceMappingStatus;

/**
 * @brief Contains all properties to manage a service deployed to a machine.
 */
typedef struct
{
    /** Hash code that uniquely defines a service */
    xmlChar *service;
    /** Name of the container to which the service is deployed */
    xmlChar *container;
    /** Name of the target machine to which the service is deployed */
    xmlChar *target;
    /** Hash code of the service that provides the container, or NULL if it uses a container provided as part of the infrastructure */
    xmlChar *container_provided_by_service;
    /** Indicates the status of the service mapping */
    ServiceMappingStatus status;
}
ServiceMapping;

/**
 * Compares two services mappings and determines their sort order.
 *
 * @param l Pointer to a service mapping instance
 * @param r Pointer to a service mapping instance
 * @return 0 if they are equal, < 0 if l comes before r, > 0 if l comes after r
 */
gint compare_service_mappings(const ServiceMapping **l, const ServiceMapping **r);

/**
 * Parses an XML document representing a snapshot mapping
 *
 * @param element XML element to parse.
 * @param userdata Arbitrary user data that is propagated to all parse functions
 * @return Pointer to a parsed interdependency mapping struct instance. It should be removed from memory with delete_snapshot_mapping()
 */
void *parse_service_mapping(xmlNodePtr element, void *userdata);

/**
 * Deletes a service mapping instance from heap memory.
 *
 * @param mapping Service mapping instance
 */
void delete_service_mapping(ServiceMapping *mapping);

/**
 * Checks whether a service mapping is valid.
 *
 * @param mapping Service mapping instance
 * @return TRUE if the service mapping is valid, else FALSE
 */
NixXML_bool check_service_mapping(const ServiceMapping *mapping);

/**
 * Prints a Nix expression representation of a service mapping
 *
 * @param file File descriptor to write to
 * @param mapping Service mapping instance
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_service_mapping_nix(FILE *file, const ServiceMapping *mapping, const int indent_level, void *userdata);

/**
 * Prints an XML representation of a service mapping.
 *
 * @param file File descriptor to write to
 * @param mapping Service mapping instance
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_service_mapping_xml(FILE *file, const ServiceMapping *mapping, const int indent_level, const char *type_property_name, void *userdata);

#endif
