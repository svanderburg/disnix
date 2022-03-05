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

#ifndef __DISNIX_ACTIVATIONMAPPING_H
#define __DISNIX_ACTIVATIONMAPPING_H
#include <glib.h>
#include <libxml/parser.h>

/**
 * @brief Contains the values that constitute a key uniquely referring to an activation mapping.
 */
typedef struct
{
    /** Hash code that uniquely defines a service */
    xmlChar *key;
    /** Target property referring to the target machine to which the service is deployed */
    xmlChar *target;
    /** Name of the container to which the service is deployed */
    xmlChar *container;
}
ActivationMappingKey;

/**
 * @brief Contains all properties to activate a specific service on a specific machine.
 * This struct maps (key,target,container) -> (service,name,type,depends_on,activated)
 */
typedef struct
{
    /** Hash code that uniquely defines a service */
    xmlChar *key;
    /** Target property referring to the target machine to which the service is deployed */
    xmlChar *target;
    /** Name of the container to which the service is deployed */
    xmlChar *container;
    /** Nix store path to the service */
    xmlChar *service;
    /* Name of the service */
    xmlChar *name;
    /** Activation type */
    xmlChar *type;
    /** Array of ActivationMappingKey items representing the inter-dependencies */
    GPtrArray *depends_on;
    /** Array of ActivationMappingKey items representing the inter-dependencies for which the ordering does not matter */
    GPtrArray *connects_to;
}
ActivationMapping;

/**
 * Creates an array with activation mappings from the corresponding sub section
 * in an XML document.
 *
 * @param element XML root element of the sub section defining the mappings
 * @return GPtrArray containing activation mappings
 */
GPtrArray *parse_activation(xmlNodePtr element);

/**
 * Deletes an array with activation mappings including its contents.
 *
 * @param activation_array Activation array to delete
 */
void delete_activation_array(GPtrArray *activation_array);

int check_activation_array(const GPtrArray *activation_array);

#endif
