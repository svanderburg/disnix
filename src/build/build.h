/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2013  Sander van der Burg
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

/**
 * Executes a distributed build. First Nix store derivation closures are copied
 * to target machines in the network. Then the remote builds are performed
 * and finally the build results are copied back to the coordinator machine.
 * 
 * @param interface Path to the client interface executable
 * @param distributed_derivation_file Path to the distributed derivation file
 * @return 0 if everything succeeds, or else a non-zero exit value
 */
int build(gchar *interface, const gchar *distributed_derivation_file);

#endif
