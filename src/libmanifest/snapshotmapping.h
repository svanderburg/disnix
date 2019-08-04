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

/**
 * Compares two snapshot mapping keys and indicates its sort order
 *
 * @param l Pointer to a snapshot mapping key
 * @param r Pointer to a snapshot mapping key
 * @return 0 if they are equal, < 0 if l comes before r, > 0 if l comes after r
 */
gint compare_snapshot_mapping_keys(const SnapshotMappingKey **l, const SnapshotMappingKey **r);

/**
 * Compares two snapshot mappings and indicates its sort order
 *
 * @param l Pointer to a snapshot mapping instance
 * @param r Pointer to a snapshot mapping instance
 * @return 0 if they are equal, < 0 if l comes before r, > 0 if l comes after r
 */
gint compare_snapshot_mapping(const SnapshotMapping **l, const SnapshotMapping **r);

/**
 * Parses an XML document representing a snapshot mapping
 *
 * @param element XML element to parse.
 * @param userdata Arbitrary user data that is propagated to all parse functions
 * @return Pointer to a parsed snapshot mapping struct instance. It should be removed from memory with delete_snapshot_mapping()
 */
void *parse_snapshot_mapping(xmlNodePtr element, void *userdata);

/**
 * Deletes a snapshot mapping instance from heap memory.
 *
 * @param mapping Snapshot mapping instance
 */
void delete_snapshot_mapping(SnapshotMapping *mapping);

/**
 * Checks whether a snapshot mapping is valid.
 *
 * @param mapping Snapshot mapping instance
 * @return TRUE if the snapshot mapping is valid, else FALSE
 */
int check_snapshot_mapping(const SnapshotMapping *mapping);

/**
 * Prints a Nix expression representation of a snapshot mapping
 *
 * @param file File descriptor to write to
 * @param mapping Inter-dependency mapping instance
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_snapshot_mapping_nix(FILE *file, const SnapshotMapping *mapping, const int indent_level, void *userdata);

/**
 * Prints an XML representation of a snapshot mapping.
 *
 * @param file File descriptor to write to
 * @param mapping Snapshot mapping instance
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_snapshot_mapping_xml(FILE *file, const SnapshotMapping *mapping, const int indent_level, const char *type_property_name, void *userdata);

/**
 * Checks whether a snapshot mapping is selected for inclusion in a deployment
 * process based on the filter criteria. Any mapping that lacks the criteria
 * will not be included.
 *
 * @param mapping A snapshot mapping instance
 * @param container_filter The name of the container to include or NULL to include any container
 * @param component_filter The name of the container to include or NULL to include any component
 * @return TRUE if the mapping is selected for inclusion, else FALSE
 */
int mapping_is_selected(const SnapshotMapping *mapping, const gchar *container_filter, const gchar *component_filter);

#endif
