/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2010  Sander van der Burg
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

#include "edgestable.h"
#include <activationmapping.h>

GHashTable *generate_edges_table(GArray *activation_array)
{    
    unsigned int i;
    
    /* Create empty hash table */    
    GHashTable *edges_table = g_hash_table_new(g_str_hash, g_str_equal);

    for(i = 0; i < activation_array->len; i++)
    {
	/* Retrieve the current mapping from the array */
	ActivationMapping *mapping = g_array_index(activation_array, ActivationMapping*, i);
	
	/* Retrieve the target property */
	gchar *target = get_target_property(mapping);
	
	/* Generate an edge table key, which consist of Nix store component:targetProperty */
	gchar *mapping_key = g_strconcat(mapping->service, ":", target, NULL);
	
	/* Retrieve the dependency array of the mapping from the edges table */
	GArray *dependency_array = g_hash_table_lookup(edges_table, mapping_key);	
	
	/* If the dependency array does not exists, create one and add it to the table */
	if(dependency_array == NULL)
	{
	    unsigned int j;
	    GArray *depends_on = mapping->depends_on;
	    
	    /* Create new dependency array */	    
	    dependency_array = g_array_new(FALSE, FALSE, sizeof(gchar*));
	    
	    /* Create a list of mapping values for each dependency */
	    for(j = 0; j < depends_on->len; j++)
	    {
		ActivationMapping lookup;
		ActivationMapping *actual_mapping;
		gchar *mapping_value, *target;
		int actual_mapping_index;
		
		/* Retrieve current dependency from the array */		
		Dependency *dependency = g_array_index(depends_on, Dependency*, j);
		
		/* Find the activation mapping in the activation array */
		lookup.service = dependency->service;
		lookup.target = dependency->target;
		actual_mapping_index = activation_mapping_index(activation_array, &lookup);
		
		/* Retrieve the actual mapping */
		actual_mapping = g_array_index(activation_array, ActivationMapping*, actual_mapping_index);
		
		/* Get the target interface */
		target = get_target_property(actual_mapping);
		
		/* Generate mapping value from the service and target property */
		mapping_value = g_strconcat(actual_mapping->service, ":", target, NULL);
		    
		/* Add mapping value to the dependency array */
	    	g_array_append_val(dependency_array, mapping_value);
	    }
	    
	    /* Associate the dependency array to the given mapping */
	    g_hash_table_insert(edges_table, mapping_key, dependency_array);	
	}
    }

    /* Return the generated egdes table */
    return edges_table;
}

void destroy_edges_table(GHashTable *edges_table)
{
    GHashTableIter iter;
    gpointer *key;
    gpointer *value;    

    g_hash_table_iter_init(&iter, edges_table);
    while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&value)) 
    {
	gchar *mapping_key = (gchar*)key;
	GArray *dependency_array = (GArray*)value;
	unsigned int i;
	
	for(i = 0; i < dependency_array->len; i++)
	{
	    gchar *dep = g_array_index(dependency_array, gchar*, i);
	    g_free(dep);
	}
	
	g_array_free(dependency_array, TRUE);
	g_free(mapping_key);
    }
        
    g_hash_table_destroy(edges_table);
}
