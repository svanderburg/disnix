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

#ifndef __DISNIX_OLDSNAPSHOTMAPPING_H
#define __DISNIX_OLDSNAPSHOTMAPPING_H
#include <glib.h>
#include <libxml/parser.h>

/**
 * @brief Contains the values that constitute a key uniquely referring to a snapshot mapping.
 */
typedef struct
{
    /** Name of the mutable component */
    xmlChar *component;

    /** Container in which the mutable component is deployed */
    xmlChar *container;

    /** Target property referring to the target machine to which the service is deployed */
    xmlChar *target;
}
OldSnapshotMappingKey;

/**
 * @brief Contains all properties to snapshot state on a specific machine.
 * This struct maps (component,container,target) -> (transferred)
 */
typedef struct
{
    /** Name of the mutable component */
    xmlChar *component;

    /** Container in which the mutable component is deployed */
    xmlChar *container;

    /** Target property referring to the target machine to which the service is deployed */
    xmlChar *target;

    /** Full Nix store path to the corresponding service */
    xmlChar *service;

    /** Activation type */
    xmlChar *type;

    /** Indicates whether the snapshot has been transferred or not */
    gboolean transferred;
}
OldSnapshotMapping;

/**
 * Creates an array with activation mappings from the corresponding sub section
 * in an XML document.
 *
 * @param element XML root element of the sub section defining the mappings
 * @return GPtrArray containing activation mappings
 */
GPtrArray *parse_old_snapshots(xmlNodePtr element);

/**
 * Deletes an array with snapshot mappings including its contents.
 *
 * @param snapshots_array Snapshots array to delete
 */
void delete_old_snapshots_array(GPtrArray *snapshots_array);

int check_old_snapshots_array(const GPtrArray *snapshots_array);

#endif
