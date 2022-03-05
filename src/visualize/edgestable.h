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

#ifndef __DISNIX_EDGESTABLE_H
#define __DISNIX_EDGESTABLE_H
#include <glib.h>

/**
 * Generates a table which associates each activation mapping of
 * a manifest to a list of its inter-dependencies
 *
 * @param service_mapping_array Array with service mappings
 * @param services_table Hash table containing the available services and their properties
 * @param targets_table Hash table with targets
 * @param ordering TRUE if strict ordering is required, else FALSE
 * @return Generated edges table
 */
GHashTable *generate_edges_table(const GPtrArray *service_mapping_array, GHashTable *services_table, GHashTable *targets_table, int ordering);

/**
 * Removes an edges table including all its contents from memory.
 *
 * @param edges_table Edges table to remove
 */
void destroy_edges_table(GHashTable *edges_table);

/**
 * Prints the edges table in dot format.
 *
 * @param edges_table Edges table to print
 * @param ordering TRUE if strict ordering is required, else FALSE
 */
void print_edges_table(GHashTable *edges_table, int ordering);

#endif
