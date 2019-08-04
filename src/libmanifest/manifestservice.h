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

#ifndef __DISNIX_MANIFESTSERVICE_H
#define __DISNIX_MANIFESTSERVICE_H
#include <stdio.h>
#include <glib.h>
#include <libxml/parser.h>

/**
 * @brief Captures all properties of a service that can be deployed to multiple targets
 */
typedef struct
{
    /* Name of the service */
    xmlChar *name;
    /** Nix store path to the built service artifact */
    xmlChar *pkg;
    /** Activation type */
    xmlChar *type;
    /* Array of inter-dependency mappings */
    GPtrArray *depends_on;
    /* Array of inter-dependency mappings for which activation ordering does not matter */
    GPtrArray *connects_to;
}
ManifestService;

/**
 * Parses an XML document representing a manifest service
 *
 * @param element XML element to parse.
 * @param userdata Arbitrary user data that is propagated to all parse functions
 * @return Pointer to a parsed manifest service struct instance. It should be removed from memory with delete_manifest_service()
 */
void *parse_manifest_service(xmlNodePtr element, void *userdata);

/**
 * Deletes a manifest service and its properties from heap memory.
 *
 * @param service A manifest service struct instance
 */
void delete_manifest_service(ManifestService *service);

/**
 * Checks whether a manifest service is valid.
 *
 * @param service A manifest service struct instance
 * @return TRUE if the manifest service is valid, else FALSE
 */
int check_manifest_service(const ManifestService *service);

/**
 * Checks whether two manifest service instances are equal.
 *
 * @param left A manifest service struct instance
 * @param right A manifest service struct instance
 * @return TRUE if the manifest services are equal, else FALSE
 */
int compare_manifest_services(const ManifestService *left, const ManifestService *right);

/**
 * Prints a Nix expression representation of a manifest service
 *
 * @param file File descriptor to write to
 * @param service A manifest service struct instance
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_manifest_service_nix(FILE *file, const ManifestService *service, const int indent_level, void *userdata);

/**
 * Prints an XML representation of a manifest service.
 *
 * @param file File descriptor to write to
 * @param service A manifest service struct instance
 * @param indent_level Specifies the indent level, or -1 to disable indentation
 * @param type_property_name Name of the type property or NULL to not display any type annotations
 * @param userdata Arbitrary user data that gets propagated to all print functions
 */
void print_manifest_service_xml(FILE *file, const ManifestService *service, const int indent_level, const char *type_property_name, void *userdata);

#endif
