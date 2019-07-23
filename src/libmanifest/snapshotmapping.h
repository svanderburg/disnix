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

#ifndef __DISNIX_SNAPSHOTMAPPING_H
#define __DISNIX_SNAPSHOTMAPPING_H
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
SnapshotMappingKey;

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

    /** Hash code that uniquely defines a service */
    xmlChar *service;

    /** Indicates whether the snapshot has been transferred or not */
    gboolean transferred;
}
SnapshotMapping;

gint compare_snapshot_mapping_keys(const SnapshotMappingKey **l, const SnapshotMappingKey **r);

gint compare_snapshot_mapping(const SnapshotMapping **l, const SnapshotMapping **r);

int mapping_is_selected(const SnapshotMapping *mapping, const gchar *container, const gchar *component);

void delete_snapshot_mapping(SnapshotMapping *mapping);

void *parse_snapshot_mapping(xmlNodePtr element, void *userdata);

int check_snapshot_mapping(const SnapshotMapping *mapping);

void print_snapshot_mapping_nix(FILE *file, const void *value, const int indent_level, void *userdata);

void print_snapshot_mapping_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata);

#endif
