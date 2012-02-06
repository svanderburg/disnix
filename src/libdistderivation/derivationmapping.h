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

#ifndef __DISNIX_DERIVATIONMAPPING_H
#define __DISNIX_DERIVATIONMAPPING_H
#include <glib.h>

/**
 * @brief Contains a mapping of a Nix store derivation to a Disnix Service target
 */
typedef struct
{
    /** Nix store derivation path */
    gchar *derivation;
    /** Address of a disnix service */
    gchar *target;
}
DerivationItem;

/**
 * Creates a new array with derivation items from a distributed derivation
 * XML file.
 *
 * @param distributed_derivation_file Path to the distributed derivation XML file
 * @return GArray with DerivationItems
 */ 
GArray *create_derivation_array(const gchar *distributed_derivation_file);

/**
 * Deletes an array with derivation items
 *
 * @param derivation_array Derivation array to delete
 */
void delete_derivation_array(GArray *derivation_array);

#endif
