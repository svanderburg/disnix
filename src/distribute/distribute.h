/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2018  Sander van der Burg
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
#include <glib.h>

/**
 * Distributes all services defined in the manifest file to target machines
 * in the network.
 *
 * @param manifest_file Path to the manifest file which maps services to machines
 * @param max_concurrent_transfers Specifies the maximum amount of concurrent transfers
 * @return 0 if everything succeeds, else a non-zero exit status
 */
int distribute(const gchar *manifest_file, const unsigned int max_concurrent_transfers);

#endif
