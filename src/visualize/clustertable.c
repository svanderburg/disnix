/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2016  Sander van der Burg
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
#include <activationmapping.h>
#include <targets.h>

static void destroy_cluster_value(gpointer data)
{
    GHashTable *container_table = (GHashTable*)data;
    g_hash_table_destroy(container_table);
}

static void destroy_container_key(gpointer data)
{
    gchar *key = (gchar*)data;
    g_free(key);
}

static void destroy_container_value(gpointer data)
{
    GPtrArray *services_array = (GPtrArray*)data;
    g_ptr_array_free(services_array, TRUE);
}

GHashTable *generate_cluster_table(GPtrArray *activation_array, GPtrArray *target_array)
{
    unsigned int i;
    
    /* Create empty hash table */
    GHashTable *cluster_table = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, destroy_cluster_value);

    /* Check all activtion mappings */
    for(i = 0; i < activation_array->len; i++)
    {
	/* Get current mapping item */
	ActivationMapping *mapping = g_ptr_array_index(activation_array, i);
	
	/* Get target property of the current mapping item */
	Target *target = find_target(target_array, mapping->target);
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
	    containers_table = g_hash_table_new_full(g_str_hash, g_str_equal, destroy_container_key, destroy_container_value);
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
