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

#include "convert-manifest.h"
#include "oldmanifest.h"
#include "distributionmapping.h"
#include "activationmapping.h"
#include "oldsnapshotmapping.h"
#include "interdependencymapping.h"
#include "manifest.h"
#include "servicemappingarray.h"
#include "snapshotmappingarray.h"
#include <targetstable.h>

static GHashTable *generate_target_mapping_table(GHashTable *targets_table)
{
    GHashTable *target_mapping_table = g_hash_table_new(g_str_hash, g_str_equal);

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, targets_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *target_key = find_target_key((Target*)value);
        g_hash_table_insert(target_mapping_table, target_key, key);
    }

    return target_mapping_table;
}

static GPtrArray *convert_dependencies(GPtrArray *dependency_array, GHashTable *target_mapping_table)
{
    GPtrArray *converted_dependency_array = g_ptr_array_new();

    if(dependency_array != NULL)
    {
        unsigned int i;

        for(i = 0; i < dependency_array->len; i++)
        {
            ActivationMappingKey *mapping_key = g_ptr_array_index(dependency_array, i);
            InterDependencyMapping *mapping = (InterDependencyMapping*)g_malloc(sizeof(InterDependencyMapping));

            gchar *target_name = g_hash_table_lookup(target_mapping_table, mapping_key->target);

            if(target_name == NULL)
            {
                g_printerr("I don't know how to map target: %s to any machine in the infrastructure model!\n", mapping_key->target);
                return NULL;
            }

            mapping->service = mapping_key->key;
            mapping->target = (xmlChar*)target_name;
            mapping->container = mapping_key->container;
            g_ptr_array_add(converted_dependency_array, mapping);
        }
    }

    return converted_dependency_array;
}

static gchar *find_service_key(GHashTable *services_table, xmlChar *pkg)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, services_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        ManifestService *service = (ManifestService*)value;
        if(xmlStrcmp(service->pkg, pkg) == 0)
            return (gchar*)key;
    }

    return NULL;
}

static Manifest *convert_manifest(OldManifest *old_manifest, GHashTable *targets_table)
{
    unsigned int i;
    Manifest *manifest = (Manifest*)g_malloc0(sizeof(Manifest));

    /* Generate target mapping table */
    GHashTable *target_mapping_table = generate_target_mapping_table(targets_table);

    /* Attach targets table */
    manifest->targets_table = targets_table;

    /* Convert distribution mappings to profile mappings */
    manifest->profile_mapping_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    for(i = 0; i < old_manifest->distribution_array->len; i++)
    {
        DistributionItem *item = g_ptr_array_index(old_manifest->distribution_array, i);
        gchar *target_name = g_hash_table_lookup(target_mapping_table, item->target);

        if(target_name == NULL)
        {
            g_printerr("I don't know how to map target: %s to any machine in the infrastructure model!\n", item->target);
            return NULL;
        }

        g_hash_table_insert(manifest->profile_mapping_table, target_name, item->profile);
    }

    /* Convert activation mappings to service mappings and services table */

    manifest->services_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    manifest->service_mapping_array = g_ptr_array_new();

    for(i = 0; i < old_manifest->activation_array->len; i++)
    {
        ActivationMapping *activation_mapping = g_ptr_array_index(old_manifest->activation_array, i);
        gchar *target_name = g_hash_table_lookup(target_mapping_table, activation_mapping->target);

        if(target_name == NULL)
        {
            g_printerr("I don't know how to map target: %s to any machine in the infrastructure model!\n", activation_mapping->target);
            return NULL;
        }

        /* Construct service */
        if(!g_hash_table_contains(manifest->services_table, (gchar*)activation_mapping->key))
        {
            ManifestService *service = (ManifestService*)g_malloc(sizeof(ManifestService));
            service->name = (xmlChar*)activation_mapping->name;
            service->type = activation_mapping->type;
            service->pkg = activation_mapping->service;
            service->connects_to = convert_dependencies(activation_mapping->connects_to, target_mapping_table);
            service->depends_on = convert_dependencies(activation_mapping->depends_on, target_mapping_table);
            g_hash_table_insert(manifest->services_table, activation_mapping->key, service);
        }

        /* Construct service mapping */
        ServiceMapping *service_mapping = (ServiceMapping*)g_malloc(sizeof(ServiceMapping));
        service_mapping->service = activation_mapping->key;
        service_mapping->container = activation_mapping->container;
        service_mapping->target = (xmlChar*)target_name;
        g_ptr_array_add(manifest->service_mapping_array, service_mapping);
    }

    /* Convert snapshot mappings */

    manifest->snapshot_mapping_array = g_ptr_array_new();

    if(old_manifest->snapshots_array != NULL)
    {
        for(i = 0; i < old_manifest->snapshots_array->len; i++)
        {
            OldSnapshotMapping *old_snapshot_mapping = g_ptr_array_index(old_manifest->snapshots_array, i);
            SnapshotMapping *snapshot_mapping = (SnapshotMapping*)g_malloc(sizeof(SnapshotMapping));

            gchar *target_name = g_hash_table_lookup(target_mapping_table, old_snapshot_mapping->target);

            if(target_name == NULL)
            {
                g_printerr("I don't know how to map target: %s to any machine in the infrastructure model!\n", old_snapshot_mapping->target);
                return NULL;
            }

            snapshot_mapping->component = old_snapshot_mapping->component;
            snapshot_mapping->container = old_snapshot_mapping->container;
            snapshot_mapping->target = (xmlChar*)target_name;
            snapshot_mapping->service = (xmlChar*)find_service_key(manifest->services_table, old_snapshot_mapping->service);
            g_ptr_array_add(manifest->snapshot_mapping_array, snapshot_mapping);
        }
    }

    return manifest;
}

int convert_manifest_file(gchar *manifest_file, gchar *infrastructure_file, const gchar *coordinator_profile_path, gchar *profile, char *default_target_property, char *default_client_interface)
{
    OldManifest *old_manifest = open_provided_or_previous_old_manifest_file(manifest_file, coordinator_profile_path, profile);

    if(old_manifest == NULL)
    {
        g_printerr("The provided manifest file is invalid!\n");
        return 1;
    }
    else
    {
        int exit_status;

        if(check_old_manifest(old_manifest))
        {
            GHashTable *targets_table = create_targets_table_from_nix(infrastructure_file, default_target_property, default_client_interface);

            if(targets_table == NULL)
            {
                g_printerr("The provided infrastructure model is invalid!\n");
                exit_status = 1;
            }
            else
            {
                 Manifest *manifest = convert_manifest(old_manifest, targets_table);
                 print_manifest_nix(stdout, manifest, 0, NULL);

                 exit_status = 0;
            }
        }
        else
        {
            g_printerr("The manifest file is invalid!\n");
            exit_status = 1;
        }

        delete_old_manifest(old_manifest);

        return exit_status;
    }
}
