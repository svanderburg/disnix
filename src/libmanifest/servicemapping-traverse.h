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
 * @brief Enumerates the possible outcomes of an operation on a service mapping
 */
typedef enum
{
    SERVICE_ERROR,
    SERVICE_IN_PROGRESS,
    SERVICE_WAIT,
    SERVICE_DONE
}
ServiceStatus;

/**
 * Pointer to a function that executes an operation to modify a service
 * mapping's state.
 *
 * @param mapping A service mapping to change the state for
 * @param service The properties of the service that is to be mapped
 * @param target The properties of the target machine where the service is mapped to
 * @param arguments Arguments to pass to the process
 * @param arguments_length Length of the arguments array
 * @return The PID of the process invoked
 */
typedef pid_t (*service_mapping_function) (ServiceMapping *mapping, ManifestService *service, Target *target, xmlChar **arguments, unsigned int arguments_length);

/**
 * Pointer to a function that gets executed when an operation on a service
 * mapping completes.
 *
 * @param mapping An service mapping to change the state for
 * @param service The properties of the service that is to be mapped
 * @param target The properties of the target machine where the service is mapped to
 * @param status Indicates whether the process terminated abnormally or not
 * @param result TRUE if the operation succeeded, else FALSE
 */
typedef void (*complete_service_mapping_function) (ServiceMapping *mapping, ManifestService *service, Target *target, ProcReact_Status status, int result);

/**
 * Pointer to a function that traverses the collection of service mappings
 * according to some strategy.
 *
 * @param unified_service_mapping_array An array of service mappings that exist in the previous and current configuration
 * @param unified_services_table A hash table of services that exist in the previous and current configuration
 * @param key The key values of a service mapping to visit
 * @param targets_table A hash table of targets
 * @param pid_table Hash table translating PIDs to service mappings
 * @param map_service_mapping Pointer to a function that executes an operation modifying the deployment state of a service mapping
 * @return Any of the activation status codes
 */
typedef ServiceStatus (*iterate_strategy_function) (GPtrArray *unified_service_mapping_array, GHashTable *unified_services_table, const InterDependencyMapping *key, GHashTable *targets_table, GHashTable *pid_table, service_mapping_function map_service_mapping);

/**
 * Searches for all the mappings in an array that have an inter-dependency
 * on the given mapping.
 *
 * @param services_table Hash table with services
 * @param service_mapping_array Array of service mappings
 * @param mapping service mapping from which to derive the interdependent mapping
 * @return Array with interdependent service mappings
 */
GPtrArray *find_interdependent_service_mappings(GHashTable *services_table, const GPtrArray *service_mapping_array, const ServiceMapping *mapping);

/**
 * Traverses a collection of service mappings by recursively visiting the
 * inter-dependencies first and then the service mapping with the provided
 * key. This strategy is, for example, useful to reliably activate services
 * without breaking dependencies.
 *
 * @param unified_service_mapping_array An array of service mappings that exist in the previous and current configuration
 * @param unified_services_table A hash table of services that exist in the previous and current configuration
 * @param key The key values of a service mapping to visit
 * @param targets_table An hash table of targets
 * @param pid_table Hash table translating PIDs to service mappings
 * @param map_service_mapping Pointer to a function that executes an operation modifying the deployment state of a service mapping
 * @return Any of the activation status codes
 */
ServiceStatus traverse_inter_dependency_mappings(GPtrArray *unified_service_mapping_array, GHashTable *unified_services_table, const InterDependencyMapping *key, GHashTable *targets_table, GHashTable *pid_table, service_mapping_function map_service_mapping);

/**
 * Traverses a collection of services mappings by recursively visting the
 * inter-dependent mappings first (reverse dependencies) and then the activation
 * with the provided key. This strategy is, for example, useful to reliably
 * deactivate services without breaking dependencies.
 *
 * @param unified_service_mapping_array An array of service mappings that exist in the previous and current configuration
 * @param unified_services_table A hash table of services that exist in the previous and current configuration
 * @param key The key values of a service mapping to visit
 * @param targets_table A hash table of targets
 * @param pid_table Hash table translating PIDs to service mappings
 * @param map_service_mapping Pointer to a function that executes an operation modifying the deployment state of a service mapping
 * @return Any of the activation status codes
 */
ServiceStatus traverse_interdependent_mappings(GPtrArray *unified_service_mapping_array, GHashTable *unified_services_table, const InterDependencyMapping *key, GHashTable *targets_table, GHashTable *pid_table, service_mapping_function map_service_mapping);

/**
 * Traverses the provided service mappings according to some strategy,
 * asynchronously executing operations for each encountered service mapping
 * that has not yet been executed. Furthermore, it also limits the amount of
 * operations executed concurrently to a specified amount per machine.
 *
 * @param mappings An array of service mappings whose state needs to be changed.
 * @param unified_service_mapping_array An array of service mappings that exist in the previous and current configuration
 * @param unified_services_table A hash table of services that exist in the previous and current configuration
 * @param targets_table A hash table of targets
 * @param iterate_strategy Pointer to a function that traverses the service mappings according to some strategy
 * @param map_service_mapping Pointer to a function that executes an operation modifying the deployment state of a service mapping
 * @param complete_service_mapping Pointer to function that gets executed when an operation on a service mapping completes
 * @return TRUE if all the service mappings' states have been successfully changed, else FALSE
 */
int traverse_service_mappings(GPtrArray *mappings, GPtrArray *unified_service_mapping_array, GHashTable *unified_services_table, GHashTable *targets_table, iterate_strategy_function iterate_strategy, service_mapping_function map_service_mapping, complete_service_mapping_function complete_service_mapping);

#endif
