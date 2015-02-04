/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2013  Sander van der Burg
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

GHashTable *generate_cluster_table(GArray *activation_array, GArray *target_array)
{
    unsigned int i;
    
    /* Create empty hash table */
    GHashTable *cluster_table = g_hash_table_new(g_str_hash, g_str_equal);

    /* Check all activtion mappings */
    for(i = 0; i < activation_array->len; i++)
    {
	/* Get current mapping item */
	ActivationMapping *mapping = g_array_index(activation_array, ActivationMapping*, i);
	
	/* Get target property of the current mapping item */
	GArray *target = get_target(target_array, mapping->target);
	gchar *target_key = get_target_key(target);
	
	/* See whether the target already exists in the table */
	GArray *services_array = g_hash_table_lookup(cluster_table, target_key);
	
	/*
	 * If the target is not yet in the table, create a new empty array
	 * of services and add it to a new entry in the hash table
	 */
	if(services_array == NULL)
	{
	    services_array = g_array_new(FALSE, FALSE, sizeof(ActivationMapping*));
	    g_hash_table_insert(cluster_table, target_key, services_array);
	}
	
	/* Append service to the array */
	g_array_append_val(services_array, mapping);
    }
    
    /* Return the generated cluster table */
    return cluster_table;
}

void destroy_cluster_table(GHashTable *cluster_table)
{
    GHashTableIter iter;
    gpointer *key;
    gpointer *value;

    g_hash_table_iter_init(&iter, cluster_table);
    while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&value)) 
    {
	GArray *services_array = (GArray*)value;
	g_array_free(services_array, TRUE);
    }
    
    g_hash_table_destroy(cluster_table);
}
