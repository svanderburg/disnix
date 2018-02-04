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

#ifndef __DISNIX_ACTIVATIONMAPPING_H
#define __DISNIX_ACTIVATIONMAPPING_H
#include <glib.h>
#include "targets.h"

/**
 * @brief Contains the values that constitute a key uniquely referring to an activation mapping.
 */
typedef struct
{
    /** Hash code that uniquely defines a service */
    gchar *key;
    /** Target property referring to the target machine to which the service is deployed */
    gchar *target;
    /** Name of the container to which the service is deployed */
    gchar *container;
}
ActivationMappingKey;

/**
 * @brief Enumeration of possible states for an activation mapping.
 */
typedef enum
{
    ACTIVATIONMAPPING_DEACTIVATED,
    ACTIVATIONMAPPING_IN_PROGRESS,
    ACTIVATIONMAPPING_ACTIVATED,
    ACTIVATIONMAPPING_ERROR
}
ActivationMappingStatus;

/**
 * @brief Contains all properties to activate a specific service on a specific machine.
 * This struct maps (key,target,container) -> (service,name,type,depends_on,activated)
 */
typedef struct
{
    /** Hash code that uniquely defines a service */
    gchar *key;
    /** Target property referring to the target machine to which the service is deployed */
    gchar *target;
    /** Name of the container to which the service is deployed */
    gchar *container;
    /** Nix store path to the service */
    gchar *service;
    /* Name of the service */
    gchar *name;
    /** Activation type */
    gchar *type;
    /** Array of ActivationMappingKey items representing the inter-dependencies */
    GPtrArray *depends_on;
    /** Array of ActivationMappingKey items representing the inter-dependencies for which the ordering does not matter */
    GPtrArray *connects_to;
    /** Indicates the status of the activation mapping */
    ActivationMappingStatus status;
}
ActivationMapping;

/**
 * @brief Enumerates the possible outcomes of an operation on activation mapping
 */
typedef enum
{
    ACTIVATION_ERROR,
    ACTIVATION_IN_PROGRESS,
    ACTIVATION_WAIT,
    ACTIVATION_DONE
}
ActivationStatus;

/**
 * Pointer to a function that executes an operation to modify an activation
 * mapping's state.
 *
 * @param mapping An activation mapping to change the state for
 * @param target The properties of the target machine where the activation is mapped to
 * @param arguments Arguments to pass to the process
 * @param arguments_length Length of the arguments array
 * @return The PID of the process invoked
 */
typedef pid_t (*map_activation_mapping_function) (ActivationMapping *mapping, Target *target, gchar **arguments, unsigned int arguments_length);

/**
 * Pointer to a function that gets executed when an operation on activation
 * mapping completes.
 *
 * @param mapping An activation mapping to change the state for
 * @param status Indicates whether the process terminated abnormally or not
 * @param result TRUE if the operation succeeded, else FALSE
 */
typedef void (*complete_activation_mapping_function) (ActivationMapping *mapping, ProcReact_Status status, int result);

/**
 * Pointer to a function that traverses the collection of activation mappings
 * according to some strategy.
 *
 * @param union_array An array of activation mappings containing mappings from the current deployment state and the desired deployment state
 * @param key The key values of an activation mapping to visit
 * @param target_array An array of target machine configurations
 * @param pid_table Hash table translating PIDs to activation mappings
 * @param map_activation_mapping Pointer to a function that executes an operation modifying the deployment state of an activation mapping
 * @return Any of the activation status codes
 */
typedef ActivationStatus (*iterate_strategy_function) (GPtrArray *union_array, const ActivationMappingKey *key, GPtrArray *target_array, GHashTable *pid_table, map_activation_mapping_function map_activation_mapping);

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

/**
 * Traverses the collection of activation mappings by recursively visiting the
 * inter-dependencies first and then the activation mapping with the provided
 * key. This strategy is, for example, useful to reliably activate services
 * without breaking dependencies.
 *
 * @param union_array An array of activation mappings containing mappings from the current deployment state and the desired deployment state
 * @param key The key values of an activation mapping to visit
 * @param target_array An array of target machine configurations
 * @param pid_table Hash table translating PIDs to activation mappings
 * @param map_activation_mapping Pointer to a function that executes an operation modifying the deployment state of an activation mapping
 * @return Any of the activation status codes
 */
ActivationStatus traverse_inter_dependency_mappings(GPtrArray *union_array, const ActivationMappingKey *key, GPtrArray *target_array, GHashTable *pid_table, map_activation_mapping_function map_activation_mapping);

/**
 * Traverses the collection of activation mappings by recursively visting the
 * inter-dependent mappings first (reverse dependencies) and then the activation
 * with the provided key. This strategy is, for example, useful to reliably
 * deactivate services without breaking dependencies.
 *
 * @param union_array An array of activation mappings containing mappings from the current deployment state and the desired deployment state
 * @param key The key values of an activation mapping to visit
 * @param target_array An array of target machine configurations
 * @param pid_table Hash table translating PIDs to activation mappings
 * @param map_activation_mapping Pointer to a function that executes an operation modifying the deployment state of an activation mapping
 * @return Any of the activation status codes
 */
ActivationStatus traverse_interdependent_mappings(GPtrArray *union_array, const ActivationMappingKey *key, GPtrArray *target_array, GHashTable *pid_table, map_activation_mapping_function map_activation_mapping);

/**
 * Traverses the provided activation mappings according to some strategy,
 * asynchronously executing operations for each encountered activation mapping
 * that has not yet been executed. Furthermore, it also limits the amount of
 * operations executed concurrently to a specified amount per machine.
 *
 * @param mappings An array of activation mappings whose state needs to be changed.
 * @param union_array An array of activation mappings containing mappings from the current deployment state and the desired deployment state
 * @param target_array An array of target machine configurations
 * @param iterate_strategy Pointer to a function that traverses the activation mappings according to some strategy
 * @param map_activation_mapping Pointer to a function that executes an operation modifying the deployment state of an activation mapping
 * @param complete_activation_mapping Pointer to function that gets executed when an operation on activation mapping completes
 * @return TRUE if all the activation mappings' states have been successfully changed, else FALSE
 */
int traverse_activation_mappings(GPtrArray *mappings, GPtrArray *union_array, GPtrArray *target_array, iterate_strategy_function iterate_strategy, map_activation_mapping_function map_activation_mapping, complete_activation_mapping_function complete_activation_mapping);

#endif
