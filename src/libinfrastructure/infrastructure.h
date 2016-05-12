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

#endif
