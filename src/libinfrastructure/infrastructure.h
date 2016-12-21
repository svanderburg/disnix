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

#ifndef __DISNIX_INFRASTRUCTURE_H
#define __DISNIX_INFRASTRUCTURE_H
#include <glib.h>
#include <xmlutil.h>
#include <procreact_pid_iterator.h>
#include <procreact_future_iterator.h>

/**
 * @brief Encapsulates a property of a machine
 */
typedef struct
{
    /** Name of the machine property */
    gchar *name;
    
    /** Value of the machine property */
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
    
    /* Contains the attribute name of the target machine */
    gchar *name;
    
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

/** Pointer to a function that executes a process for each target */
typedef pid_t (*map_target_pid_function) (void *data, Target *target, gchar *client_interface, gchar *target_key);
/** Pointer to a function that constructs a future for each target */
typedef ProcReact_Future (*map_target_future_function) (void *data, Target *target, gchar *client_interface, gchar *target_key);

/** Pointer to a function that gets executed when a process completes for a target */
typedef void (*complete_target_mapping_pid_function) (void *data, Target *target, gchar *target_key, ProcReact_Status status, int result);
/** Pointer to a function that constructs a future when a process completes for a target */
typedef void (*complete_target_mapping_future_function) (void *data, Target *target, gchar *target_key, ProcReact_Future *future, ProcReact_Status status);

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
    GPtrArray *target_array;
    /** Global key that is used if no target property is defined by the target machine */
    const gchar *target_property;
    /** Command-line tool that is invoked to reach the target machine */
    gchar *interface;
    /** Hash table keeping track with PID belongs to which target */
    GHashTable *pid_table;
    
    /** Function that executes a process for each target */
    union
    {
        /**
         * Pointer to a function that executes a process for each target
         *
         * @param data An arbitrary data structure
         * @param target A target from the target array
         * @param client_interface Executable to invoke to reach the target machine
         * @param target_key Key that uniquely identifies the machine
         * @return The PID of the spawned process
         */
        map_target_pid_function pid;
        
        /**
         * Pointer to a function that constructs a future for each target
         *
         * @param data An arbitrary data structure
         * @param target A target from the target array
         * @param client_interface Executable to invoke to reach the target machine
         * @param target_key Key that uniquely identifies the machine
         * @return A future instance
         */
        map_target_future_function future;
    }
    map_target_function;
    
    /** Function that gets executed when a process completes for a target */
    union
    {
        /**
         * Pointer to a function that gets executed when a process completes for a target
         *
         * @param data An arbitrary data structure
         * @param target A target from the target array
         * @param target_key Key that uniquely identifies the machine
         * @param status Indicates whether the process terminated abnormally or not
         * @param result TRUE if the operation succeeded, else FALSE
         */
        complete_target_mapping_pid_function pid;
        
        /**
         * Pointer to a function that constructs a future when a process completes for a target
         *
         * @param data An arbitrary data structure
         * @param target A target from the target array
         * @param target_key Key that uniquely identifies the machine
         * @param future The future that has completed
         * @param status Indicates whether the process terminated abnormally or not
         */
        complete_target_mapping_future_function future;
    }
    complete_target_mapping_function;
    
    /** Pointer to arbitrary data passed to the above functions */
    void *data;
}
TargetIteratorData;

/**
 * Creaes an array with targets from an XML document
 *
 * @param doc XML document composed by the XML parser
 * @return GPtrArray with target properties
 */
GPtrArray *create_target_array_from_doc(xmlDocPtr doc);

/**
 * Creates an array with targets from an infrastructure Nix expression
 *
 * @param infrastructure_expr Path to the infrastructure Nix expression
 * @return GPtrArray with target properties
 */
GPtrArray *create_target_array(char *infrastructure_expr);

/**
 * Deletes an array with targets
 *
 * @param target_array Array to delete
 */
void delete_target_array(GPtrArray *target_array);

/**
 * Retrieves the value of the target property that serves as the key to identify
 * the machine.
 *
 * @param target A target struct containing properties of a target machine
 * @param global_target_property Global key that is used if no target property is defined by the target machine
 * @return The key value of identifying the machine or NULL if it does not exists
 */
gchar *find_target_key(const Target *target, const gchar *global_target_property);

/**
 * Creates a new PID iterator that steps over each target and executes the
 * provided functions on start and completion.
 *
 * @param target_array Array with targets
 * @param target_property Global key that is used if no target property is defined by the target machine
 * @param interface Command-line tool that is invoked to reach the target machine
 * @param map_target Function that executes a process for each target
 * @param complete_target_mapping Function that gets executed when a process completes for a target
 * @param data Pointer to arbitrary data passed to the above functions
 * @return A PID iterator that can be used to traverse the targets
 */
ProcReact_PidIterator create_target_pid_iterator(GPtrArray *target_array, const gchar *target_property, gchar *interface, map_target_pid_function map_target, complete_target_mapping_pid_function complete_target_mapping, void *data);

/**
 * Creates a new future iterator that steps over each target and executes the
 * provided functions on start and completion.
 *
 * @param target_array Array with targets
 * @param target_property Global key that is used if no target property is defined by the target machine
 * @param interface Command-line tool that is invoked to reach the target machine
 * @param map_target Function that executes a process for each target
 * @param complete_target_mapping Function that gets executed when a process completes for a target
 * @param data Pointer to arbitrary data passed to the above functions
 * @return A future iterator that can be used to traverse the targets
 */
ProcReact_FutureIterator create_target_future_iterator(GPtrArray *target_array, const gchar *target_property, gchar *interface, map_target_future_function map_target, complete_target_mapping_future_function complete_target_mapping, void *data);

/**
 * Destroys all resources allocated with the target iterator data
 *
 * @param target_iterator_data Struct with properties that facilitate iteration over targets
 */
void destroy_target_iterator_data(TargetIteratorData *target_iterator_data);

/**
 * Checks whether all iteration steps have succeeded.
 *
 * @param target_iterator_data Struct with properties that facilitate iteration over targets
 * @return TRUE if it indicates success, else FALSE
 */
int target_iterator_has_succeeded(const TargetIteratorData *target_iterator_data);

#endif
