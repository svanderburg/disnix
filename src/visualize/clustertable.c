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

#include "clustertable.h"
#include <servicemappingarray.h>
#include <targetstable.h>

static void destroy_cluster_value(gpointer data)
{
    GHashTable *container_table = (GHashTable*)data;
    g_hash_table_destroy(container_table);
}

static void destroy_container_value(gpointer data)
{
    GPtrArray *services_array = (GPtrArray*)data;
    g_ptr_array_free(services_array, TRUE);
}

GHashTable *generate_cluster_table(GPtrArray *service_mapping_array, GHashTable *targets_table)
{
    unsigned int i;

    /* Create empty hash table */
    GHashTable *cluster_table = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, destroy_cluster_value);

    /* Check all service mappings */
    for(i = 0; i < service_mapping_array->len; i++)
    {
        /* Get current mapping item */
        ServiceMapping *mapping = g_ptr_array_index(service_mapping_array, i);

        /* Get target property of the current mapping item */
        Target *target = g_hash_table_lookup(targets_table, (gchar*)mapping->target);
        gchar *target_key = find_target_key(target);

        /* See whether the target already exists in the table */
        GHashTable *containers_table = g_hash_table_lookup(cluster_table, target_key);
        GPtrArray *services_array;
        gchar *container_key;

        /*
         * If the target is not yet in the table, create a new hashtable of containers
         * per machine
         */

        if(containers_table == NULL)
        {
            containers_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, destroy_container_value);
            g_hash_table_insert(cluster_table, target_key, containers_table);
        }

        /*
         * If the services are not yet in the containers table, add a new empty array
         * of services
         */

        container_key = g_strconcat(target_key, ":", mapping->container, NULL);
        services_array = g_hash_table_lookup(containers_table, container_key);

        if(services_array == NULL)
        {
            services_array = g_ptr_array_new();
            g_hash_table_insert(containers_table, container_key, services_array);
        }
        else
            g_free(container_key);

        /* Append service to the array */
        g_ptr_array_add(services_array, mapping);
    }

    /* Return the generated cluster table */
    return cluster_table;
}

void destroy_cluster_table(GHashTable *cluster_table)
{
    g_hash_table_destroy(cluster_table);
}

static void print_service_label(ManifestService *service)
{
    g_print("%s", service->name);

    if(g_hash_table_size(service->provides_containers_table) == 1)
    {
        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init(&iter, service->provides_containers_table);
        while(g_hash_table_iter_next(&iter, &key, &value))
            g_print(" ->\\n%s", (gchar*)key);
    }
    else if(g_hash_table_size(service->provides_containers_table) > 1)
    {
        GHashTableIter iter;
        gpointer key, value;
        int first = TRUE;

        g_print("\\n -> { ");

        g_hash_table_iter_init(&iter, service->provides_containers_table);
        while(g_hash_table_iter_next(&iter, &key, &value))
        {
            if(first)
                first = FALSE;
            else
                g_print("\\n, ");

            g_print("%s", (gchar*)key);
        }

        g_print("\\n}");
    }
}

static void print_container_table(GHashTable *services_table, GHashTable *container_table, int id, gchar *target, int no_containers)
{
    GHashTableIter iter;
    gpointer key, value;
    int count = 0;

    g_hash_table_iter_init(&iter, container_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        unsigned int i;
        GPtrArray *cluster_array = (GPtrArray*)value;

        if(!no_containers)
        {
            g_print("subgraph cluster_%d_%d {\n", id, count);
            g_print("style=filled;\n");

            if(cluster_array->len > 0)
            {
                ServiceMapping *mapping = g_ptr_array_index(cluster_array, 0);
                g_print("label=\"%s\";\n", mapping->container);
            }

            g_print("fillcolor=grey40\n");
        }

        g_print("node [style=filled,fillcolor=white,color=black];\n");

        for(i = 0; i < cluster_array->len; i++)
        {
            ServiceMapping *mapping = g_ptr_array_index(cluster_array, i);
            ManifestService *service = g_hash_table_lookup(services_table, (gchar*)mapping->service);

            g_print("\"%s:%s:%s\" [ label = \"", mapping->service, target, mapping->container);
            print_service_label(service);
            g_print("\"");

            if(g_hash_table_size(service->provides_containers_table) > 0)
                g_print(", peripheries=2");

            g_print(" ];\n");
        }

        if(!no_containers)
            g_print("}\n");

        count++;
    }
}

void print_cluster_table(GHashTable *cluster_table, GHashTable *services_table, int no_containers)
{
    GHashTableIter iter;
    gpointer key, value;
    int count = 0;

    g_hash_table_iter_init(&iter, cluster_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        gchar *target = (gchar*)key;
        GHashTable *container_table = (GHashTable*)value;

        g_print("subgraph cluster_%d {\n", count);
        g_print("style=filled;\n");
        g_print("label=\"%s\";\n", target);
        g_print("fillcolor=grey\n");

        print_container_table(services_table, container_table, count, target, no_containers);

        g_print("}\n");

        count++;
    }
}
