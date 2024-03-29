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

#ifndef __DISNIX_RUN_BUILD_H
#define __DISNIX_RUN_BUILD_H
#include <glib.h>

/**
 * Executes a distributed build. First Nix store derivation closures are copied
 * to target machines in the network. Then the remote builds are performed
 * and finally the build results are copied back to the coordinator machine.
 *
 * @param distributed_derivation_file Path to the distributed derivation file
 * @param max_concurrent_transfers Specifies the maximum amount of concurrent transfers
 * @param tmpdir Directory in which the temp files should be stored
 * @return 0 if everything succeeds, or else a non-zero exit value
 */
int run_build(const gchar *distributed_derivation_file, const unsigned int max_concurrent_transfers, char *tmpdir);

#endif
