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

#ifndef __DISNIX_DISTRIBUTE_H
#define __DISNIX_DISTRIBUTE_H
#include <manifest.h>

/**
 * Distributes the Nix store closures of all services in the manifest to the
 * target machines in the network.
 *
 * @param manifest Manifest containing all deployment information
 * @param max_concurrent_transfers Specifies the maximum amount of concurrent transfers
 * @return TRUE if all closures have been successfully transferred, else FALSE
 */
int distribute(const Manifest *manifest, const unsigned int max_concurrent_transfers);

#endif
