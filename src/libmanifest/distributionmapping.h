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

#ifndef __DISTRIBUTIONMAPPING_H
#define __DISTRIBUTIONMAPPING_H
#include <glib.h>

/**
 * Contains a mapping of a Nix profile to a disnix service target
 */
typedef struct
{
    /** Nix store path to the profile */
    gchar *profile;
    /** Address of a disnix service */
    gchar *target;
}
DistributionItem;

/**
 * Creates a new array with distribution items from a manifest file.
 *
 * @param manifest_file Path to the manifest XML file
 * @return GArray with DistributionItems
 */
GArray *generate_distribution_array(gchar *manifest_file);

/**
 * Deletes an array with distribution items.
 *
 * @param distribution_array Array with distribution items
 */
void delete_distribution_array(GArray *distribution_array);

#endif
