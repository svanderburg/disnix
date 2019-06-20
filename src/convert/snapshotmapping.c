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

#include "snapshotmapping.h"
#include <stdlib.h>
#include <libxml/parser.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <nixxml-ghashtable.h>
#include <nixxml-gptrarray.h>
#include "manifest.h"

#define min(a,b) ((a) < (b) ? (a) : (b))

static gint compare_snapshot_mapping_keys(const SnapshotMappingKey **l, const SnapshotMappingKey **r)
{
    const SnapshotMappingKey *left = *l;
    const SnapshotMappingKey *right = *r;
    
    /* Compare the component names */
    gint status = xmlStrcmp(left->component, right->component);
    
    if(status == 0)
    {
        gint status = xmlStrcmp(left->target, right->target); /* If components are equal then compare the targets */
        
        if(status == 0)
            return xmlStrcmp(left->container, right->container); /* If containers are equal then compare the containers */
        else
            return status;
    }
    else
        return status;
}

static gint compare_snapshot_mapping(const SnapshotMapping **l, const SnapshotMapping **r)
{
    return compare_snapshot_mapping_keys((const SnapshotMappingKey **)l, (const SnapshotMappingKey **)r);
}

static int mapping_is_selected(const SnapshotMapping *mapping, const gchar *container, const gchar *component)
{
    return (container == NULL || xmlStrcmp((const xmlChar*) container, mapping->container) == 0) && (component == NULL || xmlStrcmp((const xmlChar*) component, mapping->component) == 0);
}

static void delete_snapshot_mapping(SnapshotMapping *mapping)
{
    xmlFree(mapping->component);
    xmlFree(mapping->container);
    xmlFree(mapping->target);
    xmlFree(mapping->service);
    xmlFree(mapping->type);
    g_free(mapping);
}

static void *create_snapshot_mapping(xmlNodePtr element, void *userdata)
{
    return g_malloc0(sizeof(SnapshotMapping));
}

static void insert_snapshot_mapping_attributes(void *table, const xmlChar *key, void *value, void *userdata)
{
    SnapshotMapping *mapping = (SnapshotMapping*)table;

    if(xmlStrcmp(key, (xmlChar*) "component") == 0)
        mapping->component = value;
    else if(xmlStrcmp(key, (xmlChar*) "container") == 0)
        mapping->container = value;
    else if(xmlStrcmp(key, (xmlChar*) "target") == 0)
        mapping->target = value;
    else if(xmlStrcmp(key, (xmlChar*) "service") == 0)
        mapping->service = value;
    else if(xmlStrcmp(key, (xmlChar*) "type") == 0)
        mapping->type = value;
    else
        xmlFree(value);
}

static gpointer parse_snapshot_mapping(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_simple_attrset(element, userdata, create_snapshot_mapping, NixXML_parse_value, insert_snapshot_mapping_attributes);
}

static GPtrArray *filter_selected_mappings(GPtrArray *snapshots_array, const gchar *container_filter, const gchar *component_filter)
{
    if(component_filter == NULL && container_filter == NULL)
        return snapshots_array;
    else
    {
        GPtrArray *filtered_snapshots_array = g_ptr_array_new();
        unsigned int i;

        for(i = 0; i < snapshots_array->len; i++)
        {
            SnapshotMapping *mapping = g_ptr_array_index(snapshots_array, i);
            if(mapping_is_selected(mapping, container_filter, component_filter))
                g_ptr_array_add(filtered_snapshots_array, mapping);
            else
                delete_snapshot_mapping(mapping);
        }

        g_ptr_array_free(snapshots_array, TRUE);
        return filtered_snapshots_array;
    }
}

GPtrArray *parse_snapshots(xmlNodePtr element, const gchar *container_filter, const gchar *component_filter)
{
    GPtrArray *snapshots_array = NixXML_parse_g_ptr_array(element, "mapping", NULL, parse_snapshot_mapping);

    /* Sort the snapshots array */
    g_ptr_array_sort(snapshots_array, (GCompareFunc)compare_snapshot_mapping);

    /* Filter only selected mappings */
    snapshots_array = filter_selected_mappings(snapshots_array, container_filter, component_filter);

    return snapshots_array;
}

void delete_snapshots_array(GPtrArray *snapshots_array)
{
    if(snapshots_array != NULL)
    {
        unsigned int i;
    
        for(i = 0; i < snapshots_array->len; i++)
        {
            SnapshotMapping *mapping = g_ptr_array_index(snapshots_array, i);
            delete_snapshot_mapping(mapping);
        }
    
        g_ptr_array_free(snapshots_array, TRUE);
    }
}

int check_snapshots_array(const GPtrArray *snapshots_array)
{
    if(snapshots_array == NULL)
        return TRUE;
    else
    {
        unsigned int i;

        for(i = 0; i < snapshots_array->len; i++)
        {
            SnapshotMapping *mapping = g_ptr_array_index(snapshots_array, i);

            if(mapping->component == NULL || mapping->container == NULL || mapping->target == NULL || mapping->service == NULL || mapping->type == NULL)
            {
                /* Check if all mandatory properties have been provided */
                g_printerr("A mandatory property seems to be missing. Have you provided a correct\n");
                g_printerr("manifest file?\n");
                return FALSE;
            }
        }

        return TRUE;
    }
}

SnapshotMapping *find_snapshot_mapping(const GPtrArray *snapshots_array, const SnapshotMappingKey *key)
{
    SnapshotMapping **ret = bsearch(&key, snapshots_array->pdata, snapshots_array->len, sizeof(gpointer), (int (*)(const void*, const void*)) compare_snapshot_mapping);
    
    if(ret == NULL)
        return NULL;
    else
        return *ret;
}

GPtrArray *subtract_snapshot_mappings(const GPtrArray *snapshots_array1, const GPtrArray *snapshots_array2)
{
    GPtrArray *return_array = g_ptr_array_new();
    unsigned int i;
    
    for(i = 0; i < snapshots_array1->len; i++)
    {
        SnapshotMapping *mapping = g_ptr_array_index(snapshots_array1, i);
        
        if(find_snapshot_mapping(snapshots_array2, (SnapshotMappingKey*)mapping) == NULL)
            g_ptr_array_add(return_array, mapping);
    }
    
    return return_array;
}

GPtrArray *find_snapshot_mappings_per_target(const GPtrArray *snapshots_array, const gchar *target)
{
    GPtrArray *return_array = g_ptr_array_new();
    unsigned int i;
    
    for(i = 0; i < snapshots_array->len; i++)
    {
        SnapshotMapping *mapping = g_ptr_array_index(snapshots_array, i);
        
        if(xmlStrcmp(mapping->target, (const xmlChar*) target) == 0)
            g_ptr_array_add(return_array, mapping);
    }
    
    return return_array;
}

static int wait_to_complete_snapshot_item(GHashTable *pid_table, const GPtrArray *target_array, complete_snapshot_item_mapping_function complete_snapshot_item_mapping)
{
    if(g_hash_table_size(pid_table) > 0)
    {
        int wstatus;
        pid_t pid = wait(&wstatus);

        if(pid == -1)
            return FALSE;
        else
        {
            Target *target;
            ProcReact_Status status;

            /* Find the corresponding snapshot mapping and remove it from the pids table */
            SnapshotMapping *mapping = g_hash_table_lookup(pid_table, &pid);
            g_hash_table_remove(pid_table, &pid);

            /* Mark mapping as transferred to prevent it from snapshotting again */
            mapping->transferred = TRUE;

            /* Signal the target to make the CPU core available again */
            target = find_target(target_array, (gchar*)mapping->target);
            signal_available_target_core(target);

            /* Return the status */
            int result = procreact_retrieve_boolean(pid, wstatus, &status);
            complete_snapshot_item_mapping(mapping, status, result);
            return(status == PROCREACT_STATUS_OK && result);
        }
    }
    else
        return TRUE;
}

int map_snapshot_items(const GPtrArray *snapshots_array, const GPtrArray *target_array, map_snapshot_item_function map_snapshot_item, complete_snapshot_item_mapping_function complete_snapshot_item_mapping)
{
    unsigned int num_processed = 0;
    int status = TRUE;
    GHashTable *pid_table = g_hash_table_new_full(g_int_hash, g_int_equal, g_free, NULL);
    
    while(num_processed < snapshots_array->len)
    {
        unsigned int i;
    
        for(i = 0; i < snapshots_array->len; i++)
        {
            SnapshotMapping *mapping = g_ptr_array_index(snapshots_array, i);
            Target *target = find_target(target_array, (gchar*)mapping->target);
            
            if(target == NULL)
                g_print("[target: %s]: Skip state of component: %s deployed to container: %s since machine is no longer present!\n", mapping->target, mapping->component, mapping->container);
            else if(!mapping->transferred && request_available_target_core(target)) /* Check if machine has any cores available, if not wait and try again later */
            {
                gchar **arguments = generate_activation_arguments(target, (gchar*)mapping->container); /* Generate an array of key=value pairs from container properties */
                unsigned int arguments_length = g_strv_length(arguments); /* Determine length of the activation arguments array */
                pid_t pid = map_snapshot_item(mapping, target, arguments, arguments_length);
                gint *pid_ptr;
                
                /* Add pid and mapping to the hash table */
                pid_ptr = g_malloc(sizeof(gint));
                *pid_ptr = pid;
                g_hash_table_insert(pid_table, pid_ptr, mapping);
              
                /* Cleanup */
                g_strfreev(arguments);
            }
        }
    
        if(!wait_to_complete_snapshot_item(pid_table, target_array, complete_snapshot_item_mapping))
            status = FALSE;
        
        num_processed++;
    }
    
    g_hash_table_destroy(pid_table);
    return status;
}

void reset_snapshot_items_transferred_status(GPtrArray *snapshots_array)
{
    unsigned int i;

    for(i = 0; i < snapshots_array->len; i++)
    {
        SnapshotMapping *mapping = g_ptr_array_index(snapshots_array, i);
        mapping->transferred = FALSE;
    }
}

GPtrArray *open_provided_or_previous_snapshots_array(const gchar *manifest_file, const gchar *coordinator_profile_path, gchar *profile, const gchar *container, const gchar *component)
{
    Manifest *manifest = open_provided_or_previous_manifest_file(manifest_file, coordinator_profile_path, profile, MANIFEST_SNAPSHOT_FLAG, container, component);

    if(manifest == NULL)
        return NULL;
    else
    {
        GPtrArray *snapshots_array = manifest->snapshots_array;
        g_free(manifest);
        return snapshots_array;
    }
}
