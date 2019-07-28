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

#include "aggregated-manifest.h"
#include <interdependencymappingarray.h>
#include <servicemappingarray.h>
#include <snapshotmappingarray.h>
#include <profilemanifesttargettable.h>

static GPtrArray *augment_local_dependencies(GPtrArray *dependency_mapping_array, xmlChar *target_key)
{
    GPtrArray *augmented_dependency_mapping_array = g_ptr_array_new();
    unsigned int i;

    for(i = 0; i < dependency_mapping_array->len; i++)
    {
        InterDependencyMapping *mapping = g_ptr_array_index(dependency_mapping_array, i);
        InterDependencyMapping *augmented_mapping = (InterDependencyMapping*)g_malloc(sizeof(InterDependencyMapping));
        augmented_mapping->service = mapping->service;
        augmented_mapping->container = mapping->container;

        if(mapping->target == NULL)
            augmented_mapping->target = target_key;
        else
            augmented_mapping->target = mapping->target;

        g_ptr_array_add(augmented_dependency_mapping_array, augmented_mapping);
    }

    return augmented_dependency_mapping_array;
}

static void aggregate_service_tables(GHashTable *aggregated_services_table, GHashTable *services_table, xmlChar *target_key)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, services_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        ManifestService *service = (ManifestService*)value;

        if(!g_hash_table_contains(aggregated_services_table, key)) // TODO: maybe check if the value is actually the same
        {
            ManifestService *aggregated_service = (ManifestService*)g_malloc(sizeof(ManifestService));
            aggregated_service->name = service->name;
            aggregated_service->pkg = service->pkg;
            aggregated_service->type = service->type;
            aggregated_service->depends_on = augment_local_dependencies(service->depends_on, target_key);
            aggregated_service->connects_to = augment_local_dependencies(service->connects_to, target_key);
            g_hash_table_insert(aggregated_services_table, key, aggregated_service);
        }
    }
}

static void aggregate_service_mapping_array(GPtrArray *aggregated_service_mapping_array, GPtrArray *service_mapping_array, xmlChar *target_key)
{
    unsigned int i;

    for(i = 0; i < service_mapping_array->len; i++)
    {
        ServiceMapping *mapping = (ServiceMapping*)g_ptr_array_index(service_mapping_array, i);
        ServiceMapping *aggregated_mapping = (ServiceMapping*)g_malloc(sizeof(ServiceMapping));
        aggregated_mapping->service = mapping->service;
        aggregated_mapping->container = mapping->container;
        aggregated_mapping->target = target_key;

        g_ptr_array_add(aggregated_service_mapping_array, aggregated_mapping);
    }
}

static void aggregate_snapshot_mapping_array(GPtrArray *aggregated_snapshot_mapping_array, GPtrArray *snapshot_mapping_array, xmlChar *target_key)
{
    unsigned int i;

    for(i = 0; i < snapshot_mapping_array->len; i++)
    {
        SnapshotMapping *mapping = (SnapshotMapping*)g_ptr_array_index(snapshot_mapping_array, i);
        SnapshotMapping *aggregated_mapping = (SnapshotMapping*)g_malloc(sizeof(SnapshotMapping));
        aggregated_mapping->service = mapping->service;
        aggregated_mapping->component = mapping->component;
        aggregated_mapping->container = mapping->container;
        aggregated_mapping->target = target_key;

        g_ptr_array_add(aggregated_snapshot_mapping_array, aggregated_mapping);
    }
}

Manifest *aggregate_manifest(GHashTable *profile_manifest_target_table, GHashTable *targets_table)
{
    GHashTableIter iter;
    gpointer key, value;

    Manifest *manifest = (Manifest*)g_malloc(sizeof(Manifest));
    manifest->targets_table = targets_table;

    /* Merge profile manifest targets */

    manifest->profile_mapping_table = g_hash_table_new(g_str_hash, g_str_equal);
    manifest->services_table = g_hash_table_new(g_str_hash, g_str_equal);
    manifest->service_mapping_array = g_ptr_array_new();
    manifest->snapshot_mapping_array = g_ptr_array_new();

    g_hash_table_iter_init(&iter, profile_manifest_target_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        xmlChar *target_key = (xmlChar*)key;
        ProfileManifestTarget *profile_manifest_target = (ProfileManifestTarget*)value;

        g_hash_table_insert(manifest->profile_mapping_table, target_key, profile_manifest_target->profile);

        aggregate_service_tables(manifest->services_table, profile_manifest_target->profile_manifest->services_table, target_key);

        aggregate_service_mapping_array(manifest->service_mapping_array, profile_manifest_target->profile_manifest->service_mapping_array, target_key);
        g_ptr_array_sort(manifest->service_mapping_array, (GCompareFunc)compare_service_mappings);

        aggregate_snapshot_mapping_array(manifest->snapshot_mapping_array, profile_manifest_target->profile_manifest->snapshot_mapping_array, target_key);
        g_ptr_array_sort(manifest->snapshot_mapping_array, (GCompareFunc)compare_snapshot_mapping_keys);
    }

    return manifest;
}

static void delete_augmented_local_dependencies(GPtrArray *dependency_mapping_array)
{
    unsigned int i;

    for(i = 0; i < dependency_mapping_array->len; i++)
    {
        InterDependencyMapping *mapping = g_ptr_array_index(dependency_mapping_array, i);
        g_free(mapping);
    }

    g_ptr_array_free(dependency_mapping_array, TRUE);
}

void delete_aggregated_manifest(Manifest *manifest)
{
    unsigned int i;

    for(i = 0; i < manifest->snapshot_mapping_array->len; i++)
    {
        SnapshotMapping *mapping = (SnapshotMapping*)g_ptr_array_index(manifest->snapshot_mapping_array, i);
        g_free(mapping);
    }

    g_ptr_array_free(manifest->snapshot_mapping_array, TRUE);

    for(i = 0; i < manifest->service_mapping_array->len; i++)
    {
        ServiceMapping *mapping = (ServiceMapping*)g_ptr_array_index(manifest->service_mapping_array, i);
        g_free(mapping);
    }

    g_ptr_array_free(manifest->service_mapping_array, TRUE);

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, manifest->services_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        ManifestService *service = (ManifestService*)value;
        delete_augmented_local_dependencies(service->depends_on);
        delete_augmented_local_dependencies(service->connects_to);
        g_free(service);
    }

    g_hash_table_destroy(manifest->services_table);

    g_hash_table_destroy(manifest->profile_mapping_table);
    g_free(manifest);
}
