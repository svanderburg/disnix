/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2017  Sander van der Burg
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

#ifndef __DISNIX_GRAPH_H
#define __DISNIX_GRAPH_H
#include <glib.h>

/**
 * Prints a clustered graph in dot format from a manifest file to
 * the standard output.
 *
 * @param manifest_file Manifest file to visualize
 * @param coordinator_profile_path Path where the current deployment state is stored for future reference
 * @param profile Name of the distributed profile
 * @param no_containers Indicates whether to visualize the containers or not
 * @return 0 in case of success, otherwise a non-zero exit value
 */
int generate_graph(const gchar *manifest_file, const gchar *coordinator_profile_path, gchar *profile, int no_containers);

#endif
