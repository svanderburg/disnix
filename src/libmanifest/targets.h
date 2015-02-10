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

#ifndef __DISNIX_TARGETS_H
#define __DISNIX_TARGETS_H
#include <glib.h>

/**
 * Contains a property of a specific target machine.
 */
typedef struct
{
    /** Name of the property */
    gchar *name;
    
    /** Value of the property */
    gchar *value;
}
TargetProperty;

/**
 * Creates a new array with targets from a manifest file
 *
 * @param manifest_file Path to the manifest XML file
 * @return GPtrArray with targets
 */
GPtrArray *generate_target_array(const gchar *manifest_file);

/**
 * Deletes an array with targets.
 *
 * @param target_array Array with distribution items
 */
void delete_target_array(GPtrArray *target_array);

/**
 * Prints the given target array.
 *
 * @param target_array Target array to print
 */
void print_target_array(const GPtrArray *target_array);

/**
 * Retrieves a target with a specific key from the target array.
 *
 * @param target_array Array of arrays representing target machines with properties
 * @param key String referring to a target property serving as the key of the target
 * @return An array containing the properties of the machine with the given key or NULL if it cannot be found
 */
GPtrArray *find_target(const GPtrArray *target_array, const gchar *key);

/**
 * Retrieves the value of a target property with the given name.
 *
 * @param target Array containing properties of a target machine
 * @param name Name of the property to retrieve
 * @return The value of the target property or NULL if it does not exists
 */
gchar *find_target_property(const GPtrArray *target, const gchar *name);

/**
 * Retrieves the value of the target property that serves as the key to identify
 * the machine.
 *
 * @param target Array containing properties of a target machine
 * @return The key value of identifying the machine or NULL if it does not exists
 */
gchar *find_target_key(const GPtrArray *target);

/**
 * Generates a string vector with: 'name=value' pairs from the
 * target properties, which are passed to the activation module as
 * environment variables. The resulting string must be eventually be removed
 * from memory with g_strfreev()
 *
 * @param target Array with target properties
 * @return String with environment variable settings
 */
gchar **generate_activation_arguments(const GPtrArray *target);

#endif
