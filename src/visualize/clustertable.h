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

#ifndef __DISNIX_CLUSTERTABLE_H
#define __DISNIX_CLUSTERTABLE_H
#include <glib.h>

/**
 * Generates a cluster table, which contains for each target property
 * of a machine the deployed services.
 *
 * @param activation_array Array with activation mappings
 * @param target_array Array with targets
 * @return Generated cluster table
 */ 
GHashTable *generate_cluster_table(GPtrArray *activation_array, GPtrArray *target_array);

/**
 * Removes a clustered table including all its contents from memory
 *
 * @param cluster_table Clustered table to destroy
 */
void destroy_cluster_table(GHashTable *cluster_table);

#endif
