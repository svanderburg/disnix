/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2022  Sander van der Burg
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

#ifndef __DISNIX_TARGET_H
#define __DISNIX_TARGET_H
#include <libxml/parser.h>
#include <glib.h>
#include <nixxml-types.h>

/**
 * @brief Contains properties of a target machine.
 */
typedef struct
{
    /* Contains arbitrary target machine properties */
    GHashTable *properties_table;

    /* Contains container-specific configuration properties */
    GHashTable *containers_table;

    /* Contains the system architecture identifier of the system */
    xmlChar *system;

    /* Refers to the executable that must be executed to connect to the target system */
    xmlChar *client_interface;

    /* Refer to the name of the property in properties that must be used to connect to the target system */
    xmlChar *target_property;

    /* Contains the amount CPU cores this machine has */
    int num_of_cores;

    /* Contains the amount of CPU cores that are currently available */
    int available_cores;
}
Target;

/**
 * Parses an XML document representing a target.
 *
 * @param element XML element to parse.
 * @param userdata Arbitrary user data that is propagated to all parse functions
 * @return Pointer to a parsed target struct instance. It should be removed from memory with delete_target()
 */
void *parse_target(xmlNodePtr element, void *userdata);

/**
 * Deletes a target and its properties from heap memory.
 *
 * @param target A target struct instance
 */
void delete_target(Target *target);

/**
 * Checks the properties of a target for validity
 *
 * @param target Target struct instance
 */
NixXML_bool check_target(const Target *target);

/**
 * Checks whether two target struct instances have the same properties
 *
 * @param left An instance of a target struct
 * @param right An instance of a target struct
 * @return TRUE if they have the same properties, else FALSE
 */
NixXML_bool compare_targets(const Target *left, const Target *right);

/**
 * Prints a Nix expression representation of a target.
 *
 * @param file File descriptor to write to
 * @param target Target struct instance
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_target_nix(FILE *file, const Target *target, const int indent_level, void *userdata);

/**
 * Prints an XML representation of a target.
 *
 * @param file File descriptor to write to
 * @param target Target struct instance
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_target_xml(FILE *file, const Target *target, const int indent_level, const char *type_property_name, void *userdata);

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
 * from memory with NixXML_delete_env_variable_array()
 *
 * @param target Struct with target properties
 * @param container_name Name of the container to deploy to
 * @return String with environment variable settings
 */
xmlChar **generate_activation_arguments_for_target(const Target *target, const gchar *container_name);

/**
 * Requests a CPU core for deployment utilisation.
 *
 * @param target A target struct containing properties of a target machine
 * @return TRUE if a CPU core is allocated, FALSE if none is available
 */
NixXML_bool request_available_target_core(Target *target);

/**
 * Signals the availability of an additional CPU core for deployment utilisation.
 *
 * @param target A target struct containing properties of a target machine
 */
void signal_available_target_core(Target *target);

#endif
