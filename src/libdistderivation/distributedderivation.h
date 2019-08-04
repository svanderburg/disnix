/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2019  Sander van der Burg
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

#ifndef __DISNIX_DISTRIBUTEDDERIVATION_H
#define __DISNIX_DISTRIBUTEDDERIVATION_H
#include <glib.h>

/**
 * @brief Contains all properties of a distributed derivation file that maps store derivation closures to machines
 *
 * A distributed derivation consists of an array mapping intra-dependency closures
 * of store derivations to interfaces and an array of interfaces used to connect
 * to the remote machines.
 */
typedef struct
{
    /** Array of store derivations to be built on a target machine */
    GPtrArray *derivation_mapping_array;

    /** Hash table containing the available interface properties of each target machine */
    GHashTable *interfaces_table;
}
DistributedDerivation;

/**
 * Composes a distributed derivation struct from a distributed derivation file.
 *
 * @param distributed_derivation_file Manifest file to open
 * @return A distributed derivation struct or NULL if an error occurred
 */
DistributedDerivation *create_distributed_derivation(const gchar *distributed_derivation_file);

/**
 * Deletes a distributed derivation struct from heap memory.
 *
 * @param distributed_derivation Distributed derivation to delete
 */
void delete_distributed_derivation(DistributedDerivation *distributed_derivation);

/**
 * Checks whether a distributed derivation is valid.
 *
 * @param distributed_derivation Distributed derivation to check
 * @return TRUE if the distributed derivation is valid, else FALSE
 */
int check_distributed_derivation(const DistributedDerivation *distributed_derivation);

#endif
