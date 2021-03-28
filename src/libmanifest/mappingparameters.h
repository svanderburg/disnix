/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2021  Sander van der Burg
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

#ifndef __DISNIX_MAPPINGPARAMETERS_H
#define __DISNIX_MAPPINGPARAMETERS_H
#include <glib.h>
#include <libxml/parser.h>
#include "manifestservice.h"
#include "target.h"

typedef struct
{
    ManifestService *service;
    ManifestService *container_service;
    xmlChar *type;
    xmlChar **arguments;
    unsigned int arguments_size;
}
MappingParameters;

MappingParameters create_mapping_parameters(const xmlChar *service, const xmlChar *container, const xmlChar *target_name, const xmlChar *container_provided_by_service, GHashTable *services_table, Target *target);

void destroy_mapping_parameters(MappingParameters *params);

#endif
