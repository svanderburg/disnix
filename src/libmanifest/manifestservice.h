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

#ifndef __DISNIX_MANIFESTSERVICE_H
#define __DISNIX_MANIFESTSERVICE_H
#include <stdio.h>
#include <glib.h>
#include <libxml/parser.h>

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

void *parse_manifest_service(xmlNodePtr element, void *userdata);

void delete_manifest_service(ManifestService *service);

int check_manifest_service(const ManifestService *service);

int compare_manifest_services(const ManifestService *left, const ManifestService *right);

void print_manifest_service_nix(FILE *file, const ManifestService *service, const int indent_level, void *userdata);

void print_manifest_service_xml(FILE *file, const ManifestService *service, const int indent_level, const char *type_property_name, void *userdata);

#endif
