/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2019  Sander van der Burg
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

#ifndef __DISNIX_SERVICEMAPPINGARRAY_H
#define __DISNIX_SERVICEMAPPINGARRAY_H
#include <glib.h>
#include <procreact_pid.h>
#include <libxml/parser.h>
#include "manifestservice.h"
#include "interdependencymappingarray.h"

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
 * @brief Contains all properties to map a specific service on a specific machine (by executing a specified activity).
 */
typedef struct
{
    /** Hash code that uniquely defines a service */
    xmlChar *service;
    /** Name of the container to which the service is deployed */
    xmlChar *container;
    /** Name of the target machine to which the service is deployed */
    xmlChar *target;
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
 * Creates an array with service mappings from the corresponding sub section
 * in an XML document. The resulting array must be freed with
 * delete_service_mapping_array() when it is not need anymore.
 *
 * @param element XML root element of the sub section defining the mappings
 * @param userdata Arbitrary user data that is propagated to all parse functions
 * @return GPtrArray containing service mappings
 */
GPtrArray *parse_service_mapping_array(xmlNodePtr element, void *userdata);

/**
 * Deletes an array with service mappings including its contents.
 *
 * @param service_mapping_array A GPtrArray with service mappings
 */
void delete_service_mapping_array(GPtrArray *service_mapping_array);

/**
 * Checks whether an array of service mappings is valid.
 *
 * @param service_mapping_array A GPtrArray with service mappings
 * @return TRUE when the content is valid, else FALSE
 */
int check_service_mapping_array(const GPtrArray *service_mapping_array);

/**
 * Compares two service mapping arrays and checks whether they have the same content
 *
 * @param service_mapping_array1 A GPtrArray with service mappings
 * @param service_mapping_array2 A GPtrArray with service mappings
 * @return TRUE if the arrays have equal content, else FALSE
 */
int compare_service_mapping_arrays(const GPtrArray *service_mapping_array1, const GPtrArray *service_mapping_array2);

/**
 * Prints a Nix expression representation of a service mapping array.
 *
 * @param file File descriptor to write to
 * @param service_mapping_array A GPtrArray with service mappings
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_service_mapping_array_nix(FILE *file, const GPtrArray *service_mapping_array, const int indent_level, void *userdata);

/**
 * Prints an XML representation of a service mapping array.
 *
 * @param file File descriptor to write to
 * @param service_mapping_array A GPtrArray with service mappings
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_service_mapping_array_xml(FILE *file, const GPtrArray *service_mapping_array, const int indent_level, const char *type_property_name, void *userdata);

/**
 * Returns the service mapping with the given key in the service mapping array.
 *
 * @param service_mapping_array Service mapping array
 * @param key Key of the service mapping to find
 * @return The service mapping with the specified keys, or NULL if it cannot be found
 */
ServiceMapping *find_service_mapping(const GPtrArray *service_mapping_array, const InterDependencyMapping *key);

/**
 * Returns the intersection of the two given service arrays.
 * The array that is returned contains pointers to elements in
 * both left and right, so it should be free with g_ptr_array_free().
 *
 * @param left A GPtrArray with service mappings
 * @param right A GPtrArray with service mappings
 * @return Array with service mappings both in left and right
 */
GPtrArray *intersect_service_mapping_array(const GPtrArray *left, const GPtrArray *right);

/**
 * Returns the union of left and right using the intersection,
 * and marks all the service mappings in left as inactive
 * and service mappings in right as active.
 *
 * @param left A GPtrArray with service mappings
 * @param right A GPtrArray with service mappings
 * @param intersect Intersection of left and right
 * @return Array with service mappings in both left and right,
 *         marked as active and inactive
 */
GPtrArray *unify_service_mapping_array(GPtrArray *left, GPtrArray *right, const GPtrArray *intersect);

/**
 * Returns a new array in which the service mappings in
 * right and substracted from left.
 * The array that is returned contains pointers to elements in
 * left, so it should be free with g_ptr_array_free().
 *
 * @param left A GPtrArray with service mappings
 * @param right A GPtrArray with service mappings
 * @return Array with right substracted from left.
 */
GPtrArray *substract_service_mapping_array(const GPtrArray *left, const GPtrArray *right);

#endif
