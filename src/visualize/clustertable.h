/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2021  Sander van der Burg
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
 * @param service_mapping_array Array with service mappings
 * @param targets_table Hash table with targets
 * @return Generated cluster table
 */
GHashTable *generate_cluster_table(GPtrArray *service_mapping_array, GHashTable *targets_table);

/**
 * Removes a clustered table including all its contents from memory
 *
 * @param cluster_table Clustered table to destroy
 */
void destroy_cluster_table(GHashTable *cluster_table);

/**
 * Prints the cluster table in dot format.
 *
 * @param cluster_table Clustered table to print
 * @param services_table Hash table containing the available services and their properties
 * @param no_containers Indicates whether not to visualize the containers
 */
void print_cluster_table(GHashTable *cluster_table, GHashTable *services_table, int no_containers);

#endif
