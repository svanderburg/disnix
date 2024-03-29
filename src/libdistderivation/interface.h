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

#ifndef __DISNIX_INTERFACE_H
#define __DISNIX_INTERFACE_H
#include <glib.h>
#include <libxml/parser.h>
#include <nixxml-types.h>

/**
 * @brief Contains properties to interface with a target machine for building
 */
typedef struct
{
    /** Target property referring to the target machine to which the service is deployed */
    gchar *target_address;

    /** Executable that needs to be run to connect to the remote machine */
    gchar *client_interface;
}
Interface;

/**
 * Parses an interface from a sub section of an XML document.
 *
 * @param element Root XML element of the sub section that defines an interface
 * @param userdata Arbitrary user data that is propagated to all parse functions
 * @return A pointer to an interface
 */
void *parse_interface(xmlNodePtr element, void *userdata);

/**
 * Deletes an interface struct instance from heap memory.
 *
 * @param interface An interface struct instance
 */
void delete_interface(Interface *interface);

/**
 * Checks whether an interface struct instance is valid.
 *
 * @param interface An interface struct instance
 * @return TRUE if the interface is valid, else FALSE
 */
NixXML_bool check_interface(const Interface *interface);

#endif
