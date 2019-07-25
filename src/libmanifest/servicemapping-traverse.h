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

#ifndef __DISNIX_SERVICEMAPPING_TRAVERSE_H
#define __DISNIX_SERVICEMAPPING_TRAVERSE_H
#include <glib.h>
#include <targetstable.h>
#include "servicestable.h"
#include "servicemappingarray.h"
#include "interdependencymappingarray.h"

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
typedef pid_t (*service_mapping_function) (ServiceMapping *mapping, ManifestService *service, Target *target, xmlChar **arguments, unsigned int arguments_length);

/**
 * Pointer to a function that gets executed when an operation on activation
 * mapping completes.
 *
 * @param mapping An activation mapping to change the state for
 * @param status Indicates whether the process terminated abnormally or not
 * @param result TRUE if the operation succeeded, else FALSE
 */
typedef void (*complete_service_mapping_function) (ServiceMapping *mapping, Target *target, ProcReact_Status status, int result);

/**
 * Pointer to a function that traverses the collection of activation mappings
 * according to some strategy.
 *
 * @param union_array An array of activation mappings containing mappings from the current deployment state and the desired deployment state
 * @param key The key values of an activation mapping to visit
 * @param targets_table A hash table of targets
 * @param pid_table Hash table translating PIDs to activation mappings
 * @param map_activation_mapping Pointer to a function that executes an operation modifying the deployment state of an activation mapping
 * @return Any of the activation status codes
 */
typedef ServiceStatus (*iterate_strategy_function) (GPtrArray *union_service_mapping, GHashTable *union_services_table, const InterDependencyMapping *key, GHashTable *targets_table, GHashTable *pid_table, service_mapping_function map_service_mapping);

/**
 * Searches for all the mappings in an array that have an inter-dependency
 * on the given mapping.
 *
 * @param service_mapping_array Array of activation mappings
 * @param mapping Activation mapping from which to derive the
 *                interdependent mapping
 * @return Array with interdependent activation mappings
 */
GPtrArray *find_interdependent_service_mappings(GHashTable *services_table, const GPtrArray *service_mapping_array, const ServiceMapping *mapping);

/**
 * Traverses the collection of activation mappings by recursively visiting the
 * inter-dependencies first and then the activation mapping with the provided
 * key. This strategy is, for example, useful to reliably activate services
 * without breaking dependencies.
 *
 * @param union_array An array of activation mappings containing mappings from the current deployment state and the desired deployment state
 * @param key The key values of an activation mapping to visit
 * @param targets_table An hash table of targets
 * @param pid_table Hash table translating PIDs to activation mappings
 * @param map_activation_mapping Pointer to a function that executes an operation modifying the deployment state of an activation mapping
 * @return Any of the activation status codes
 */
ServiceStatus traverse_inter_dependency_mappings(GPtrArray *union_array, GHashTable *union_services_table, const InterDependencyMapping *key, GHashTable *targets_table, GHashTable *pid_table, service_mapping_function map_service_mapping);

/**
 * Traverses the collection of activation mappings by recursively visting the
 * inter-dependent mappings first (reverse dependencies) and then the activation
 * with the provided key. This strategy is, for example, useful to reliably
 * deactivate services without breaking dependencies.
 *
 * @param union_array An array of activation mappings containing mappings from the current deployment state and the desired deployment state
 * @param key The key values of an activation mapping to visit
 * @param targets_table A hash table of targets
 * @param pid_table Hash table translating PIDs to activation mappings
 * @param map_activation_mapping Pointer to a function that executes an operation modifying the deployment state of an activation mapping
 * @return Any of the activation status codes
 */
ServiceStatus traverse_interdependent_mappings(GPtrArray *union_array, GHashTable *union_services_table, const InterDependencyMapping *key, GHashTable *targets_table, GHashTable *pid_table, service_mapping_function map_service_mapping);

/**
 * Traverses the provided activation mappings according to some strategy,
 * asynchronously executing operations for each encountered activation mapping
 * that has not yet been executed. Furthermore, it also limits the amount of
 * operations executed concurrently to a specified amount per machine.
 *
 * @param mappings An array of activation mappings whose state needs to be changed.
 * @param union_array An array of activation mappings containing mappings from the current deployment state and the desired deployment state
 * @param targets_table A hash table of targets
 * @param iterate_strategy Pointer to a function that traverses the activation mappings according to some strategy
 * @param map_activation_mapping Pointer to a function that executes an operation modifying the deployment state of an activation mapping
 * @param complete_activation_mapping Pointer to function that gets executed when an operation on activation mapping completes
 * @return TRUE if all the activation mappings' states have been successfully changed, else FALSE
 */
int traverse_service_mappings(GPtrArray *mappings, GPtrArray *union_array, GHashTable *union_services_table, GHashTable *targets_table, iterate_strategy_function iterate_strategy, service_mapping_function map_service_mapping, complete_service_mapping_function complete_service_mapping);

#endif
