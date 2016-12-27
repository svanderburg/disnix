/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2016  Sander van der Burg
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
#include <procreact_pid_iterator.h>

/**
 * @brief Contains a property of a specific target machine.
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
 * @brief Contains properties of a container belonging to a machine.
 */
typedef struct
{
    /** Name of the container */
    gchar *name;
    
    /** Contains the properties of the container */
    GPtrArray *properties;
}
Container;

/**
 * @brief Contains properties of a target machine.
 */
typedef struct
{
    /* Contains arbitrary target machine properties */
    GPtrArray *properties;
    
    /* Contains container-specific configuration properties */
    GPtrArray *containers;
    
    /* Contains the system architecture identifier of the system */
    gchar *system;
    
    /* Refers to the executable that must be executed to connect to the target system */
    gchar *client_interface;
    
    /* Refer to the name of the property in properties that must be used to connect to the target system */
    gchar *target_property;
    
    /* Contains the amount CPU cores this machine has */
    int num_of_cores;
    
    /* Contains the amount of CPU cores that are currently available */
    int available_cores;
}
Target;

typedef pid_t (*map_target_function) (void *data, Target *target);

typedef void (*complete_target_mapping_function) (void *data, Target *target, ProcReact_Status status, int result);

/**
 * @brief Iterator that can be used to execute a process for each target
 */
typedef struct
{
    /** Indicates which element in the array to process */
    unsigned int index;
    /** Contains the length of the array */
    unsigned int length;
    /** Indicates the success status of the iteration */
    int success;
    /** Array with targets */
    const GPtrArray *target_array;
    /** Hash table keeping track with PID belongs to which target */
    GHashTable *pid_table;
    map_target_function map_target;
    complete_target_mapping_function complete_target_mapping;
    void *data;
}
TargetIteratorData;

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
 * @param target_array Array with target items
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
 * @return An target struct containing the properties of the machine with the given key or NULL if it cannot be found
 */
Target *find_target(const GPtrArray *target_array, const gchar *key);

/**
 * Retrieves the value of a target property with the given name.
 *
 * @param target Target struct containing properties of a target machine
 * @param name Name of the property to retrieve
 * @return The value of the target property or NULL if it does not exists
 */
gchar *find_target_property(const Target *target, const gchar *name);

/**
 * Retrieves the value of the target property that serves as the key to identify
 * the machine.
 *
 * @param target A target struct containing properties of a target machine
 * @return The key value of identifying the machine or NULL if it does not exists
 */
gchar *find_target_key(const Target *target);

/**
 * Generates a string vector with: 'name=value' pairs from the
 * target properties, which are passed to the activation module as
 * environment variables. The resulting string must be eventually be removed
 * from memory with g_strfreev()
 *
 * @param target Struct with target properties
 * @param container_name Name of the container to deploy to
 * @return String with environment variable settings
 */
gchar **generate_activation_arguments(const Target *target, const gchar *container_name);

/**
 * Requests a CPU core for deployment utilisation.
 *
 * @param target A target struct containing properties of a target machine
 * @return TRUE if a CPU core is allocated, FALSE if none is available
 */
int request_available_target_core(Target *target);

/**
 * Signals the availability of an additional CPU core for deployment utilisation.
 *
 * @param target A target struct containing properties of a target machine
 */
void signal_available_target_core(Target *target);

ProcReact_PidIterator create_target_iterator(const GPtrArray *target_array, map_target_function map_target, complete_target_mapping_function complete_target_mapping, void *data);

void destroy_target_iterator(ProcReact_PidIterator *iterator);

int target_iterator_has_succeeded(const ProcReact_PidIterator *iterator);

#endif
