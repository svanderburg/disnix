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

#ifndef __DISNIX_MANIFEST_H
#define __DISNIX_MANIFEST_H
#include <glib.h>

/**
 * A deployment manifest consists of an array mapping intra-dependency closures
 * to targets, an array of services that need to be activated on a specific
 * target and array of available target machines
 */
typedef struct
{
    /** Array mapping intra-dependency closures to machines */
    GPtrArray *distribution_array;
    
    /** Array of services to be activated on a target machine */
    GPtrArray *activation_array;
    
    /** Array containing the available target machines */
    GPtrArray *target_array;
}
Manifest;

/**
 * Composes a manifest struct from a manifest file.
 *
 * @param manifest_file Manifest file to open
 * @return A manifest struct or NULL if an error occurred
 */
Manifest *create_manifest(const gchar *manifest_file);

/**
 * Deletes a manifest struct from heap memory.
 *
 * @param manifest Manifest to delete
 */
void delete_manifest(Manifest *manifest);

#endif
