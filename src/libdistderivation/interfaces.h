/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2017  Sander van der Burg
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

#ifndef __DISNIX_INTERFACES_H
#define __DISNIX_INTERFACES_H
#include <glib.h>

/**
 * @brief Contains properties to interface with a target machine for building
 */
typedef struct
{
    /** Target property referring to the target machine to which the service is deployed */
    gchar *target;
    
    /** Executable that needs to be run to connect to the remote machine */
    gchar *clientInterface;
}
Interface;

/**
 * Creates a new array with interfaces from a distributed derivation file
 *
 * @param distributed_derivation_file Path to the distributed derivation XML file
 * @return GPtrArray with targets
 */
GPtrArray *create_interface_array(const gchar *distributed_derivation_file);

/**
 * Deletes an array with interfaces.
 *
 * @param interface_array Array with interfaces
 */
void delete_interface_array(GPtrArray *interface_array);

/**
 * Retrieves an interface with a specific key from the interfaces array.
 *
 * @param interface_array Array of arrays representing interfaces
 * @param key String referring to a target property serving as the key of the target
 * @return An interfaces struct containing the properties of the machine with the given key or NULL if it cannot be found
 */
Interface *find_interface(const GPtrArray *interface_array, const gchar *key);

#endif
