/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2020  Sander van der Burg
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

#ifndef __DISNIX_BUILD_H
#define __DISNIX_BUILD_H
#include <glib.h>
#include <distributedderivation.h>

/**
 * Delegates all store derivations to the remote machines and retrieves their build results.
 *
 * @param distributed_derivation Configuration specifying a mapping between store derivations and machines
 * @param max_concurrent_transfers Specifies the maximum amount of concurrent transfers
 * @return TRUE if all the remote builds succeed, else FALSE
 */
int build(DistributedDerivation *distributed_derivation, const unsigned int max_concurrent_transfers);

#endif
