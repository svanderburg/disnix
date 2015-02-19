/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2015  Sander van der Burg
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

typedef struct
{
    gchar *name;
    gchar *value;
}
TargetProperty;

/**
 * Creates an array with target properties from an infrastructure Nix expression
 *
 * @param infrastructure_expr Path to the infrastructure Nix expression
 * @return GPtrArray with target properties
 */
GPtrArray *create_target_array(char *infrastructure_expr);

/**
 * Deletes an array with target properties
 *
 * @param target_array Array to delete
 */
void delete_target_array(GPtrArray *target_array);

/**
 * Retrieves the value of the target property that serves as the key to identify
 * the machine.
 *
 * @param target A target array containing properties of a target machine
 * @param global_target_property Global key that is used if no target property is defined by the target machine
 * @return The key value of identifying the machine or NULL if it does not exists
 */
gchar *find_target_key(const GPtrArray *target, const gchar *global_target_property);

/**
 * Retrieves the client interface property of a target machine.
 *
 * @param target A target array containing properties of a target machine
 * @return The client interface property or NULL if it does not exists
 */
gchar *find_client_interface(const GPtrArray *target);

#endif
