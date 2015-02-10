/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2013  Sander van der Burg
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

#ifndef __DISNIX_ACTIVATIONMAPPING_H
#define __DISNIX_ACTIVATIONMAPPING_H
#include <glib.h>

/**
 * Contains the values that constitute a key uniquely referring to an activation
 * mapping.
 */
typedef struct
{
    /** Hash code that uniquely defines a service */
    gchar *key;
    /** Target property referring to the target machine to which the service is deployed */
    gchar *target;
}
ActivationMappingKey;

/**
 * Contains all properties to activate a specific service on
 * a specific machine. This struct maps (key,target) ->
 * (service,name,targetProperty,type,depends_on,activated)
 */
typedef struct
{
    /** Hash code that uniquely defines a service */
    gchar *key;
    /** Target property referring to the target machine to which the service is deployed */
    gchar *target;
    /** Nix store path to the service */
    gchar *service;
    /* Name of the service */
    gchar *name;
    /** Property used to connect to the disnix service */
    gchar *targetProperty;
    /** Activation type */
    gchar *type;
    /** Array of ActivationMappingKey items representing the inter-dependencies */
    GPtrArray *depends_on;
    /** Indicated whether the service is activated or not */
    gboolean activated;
}
ActivationMapping;

/**
 * Creates an array with activation mappings from a manifest XML file.
 *
 * @param manifest_file Path to the manifest XML file
 * @return GPtrArray containing activation mappings
 */
GPtrArray *create_activation_array(const gchar *manifest_file);

/**
 * Deletes an array with activation mappings including its contents.
 *
 * @param activation_array Activation array to delete
 */
void delete_activation_array(GPtrArray *activation_array);

/**
 * Returns the activation mapping with the given key in the activation array.
 *
 * @param activation_array Activation array
 * @param key Key of the activation mapping to find
 * @return The activation mapping with the specified keys, or NULL if it cannot be found
 */
ActivationMapping *find_activation_mapping(const GPtrArray *activation_array, const ActivationMappingKey *key);

/**
 * Returns the dependency with the given keys in the dependsOn array.
 *
 * @param depends_on dependsOn array
 * @param key Key of the dependency to find
 * @return The dependency mapping with the specified keys, or NULL if it cannot be found
 */
ActivationMappingKey *find_dependency(const GPtrArray *depends_on, const ActivationMappingKey *key);

/**
 * Returns the intersection of the two given arrays.
 * The array that is returned contains pointers to elements in
 * both left and right, so it should be free with g_ptr_array_free().
 *
 * @param left Array with activation mappings
 * @param right Array with activation mappings
 * @return Array with activation mappings both in left and right
 */
GPtrArray *intersect_activation_array(const GPtrArray *left, const GPtrArray *right);

/**
 * Returns the union of left and right using the intersection,
 * and marks all the activation mappings in left as inactive
 * and activation mappings in right as active.
 *
 * @param left Array with activation mappings
 * @param right Array with activation mappings 
 * @param intersect Intersection of left and right
 * @return Array with activation mappings in both left and right,
 *         marked as active and inactive
 */
GPtrArray *union_activation_array(GPtrArray *left, GPtrArray *right, const GPtrArray *intersect);

/**
 * Returns a new array in which the activation mappings in
 * right and substracted from left.
 * The array that is returned contains pointers to elements in
 * left, so it should be free with g_ptr_array_free().
 *
 * @param left Array with activation mappings
 * @param right Array with activation mappings
 * @return Array with right substracted from left.
 */
GPtrArray *substract_activation_array(const GPtrArray *left, const GPtrArray *right);

/**
 * Searches for all the mappings in an array that have an inter-dependency
 * on the given mapping.
 *
 * @param activation_array Array of activation mappings
 * @param mapping Activation mapping from which to derive the
 *                interdependent mapping
 * @return Array with interdependent activation mappings
 */
GPtrArray *find_interdependent_mappings(const GPtrArray *activation_array, const ActivationMapping *mapping);

/**
 * Prints the given activation array.
 *
 * @param activation_array Activation array to print
 */
void print_activation_array(const GPtrArray *activation_array);

#endif
