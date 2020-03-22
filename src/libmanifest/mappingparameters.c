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

#include "mappingparameters.h"
#include "manifestservicestable.h"
#include <nixxml-generate-env.h>

static xmlChar *determine_type(const ManifestService *service, const ManifestService *container_service)
{
    if(container_service == NULL)
        return service->type;
    else
        return container_service->pkg;
}

static xmlChar **generate_activation_arguments(const ManifestService *container_service, const Target *target, const gchar *container_name)
{
    if(container_service == NULL)
        return generate_activation_arguments_for_target(target, container_name);
    else
        return generate_activation_arguments_for_container_service(container_service, container_name);
}

MappingParameters create_mapping_parameters(const xmlChar *service, const xmlChar *container, const xmlChar *target_name, GHashTable *services_table, Target *target)
{
    MappingParameters params;
    params.service = g_hash_table_lookup(services_table, (const gchar*)service);
    params.container_service = find_container_service_dependency(services_table, params.service, container, target_name);
    params.type = determine_type(params.service, params.container_service);

    params.arguments = generate_activation_arguments(params.container_service, target, (const gchar*)container); /* Generate an array of key=value pairs from container properties */
    params.arguments_size = g_strv_length((gchar**)params.arguments); /* Determine length of the activation arguments array */

    return params;
}

void destroy_mapping_parameters(MappingParameters *params)
{
    NixXML_delete_env_variable_array(params->arguments);
}
