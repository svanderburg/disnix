/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2011  Sander van der Burg
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

#ifndef __DISNIX_TARGETS_H
#define __DISNIX_TARGETS_H
#include <glib.h>

/**
 * Creates a new array with targets from a manifest file
 *
 * @param manifest_file Path to the manifest XML file
 * @return GArray with targets
 */
GArray *generate_target_array(const gchar *manifest_file);

/**
 * Deletes an array with distribution items.
 *
 * @param distribution_array Array with distribution items
 */
void delete_target_array(GArray *target_array);

/**
 * Returns the index of the given target in the target array
 * or -1 if the target does not exists.
 *
 * @param target_array Array of possible targets
 * @param target Target to lookup
 * @return Index of target in the array, or -1 if not found
 */
int target_index(const GArray *target_array, const gchar *target);

#endif
