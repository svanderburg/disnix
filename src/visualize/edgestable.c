/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2020  Sander van der Burg
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
#include <servicemappingarray.h>
#include <targetstable.h>

static void destroy_value(gpointer data)
{
    GPtrArray *dependency_array = (GPtrArray*)data;
    unsigned int i;

    for(i = 0; i < dependency_array->len; i++)
    {
        gchar *dep = g_ptr_array_index(dependency_array, i);
        g_free(dep);
    }

    g_ptr_array_free(dependency_array, TRUE);
}

static gchar *compose_mapping_key(ServiceMapping *mapping, gchar *target_key)
{
    return g_strconcat((gchar*)mapping->service, ":", target_key, ":", (gchar*)mapping->container, NULL);
}

GHashTable *generate_edges_table(const GPtrArray *service_mapping_array, GHashTable *services_table, GHashTable *targets_table, int ordering)
{
    unsigned int i;

    /* Create empty hash table */
    GHashTable *edges_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, destroy_value);

    for(i = 0; i < service_mapping_array->len; i++)
    {
	/* Retrieve the current mapping from the array */
	ServiceMapping *mapping = g_ptr_array_index(service_mapping_array, i);
	ManifestService *service = g_hash_table_lookup(services_table, mapping->service);

	/* Retrieve the target property */
	Target *target = g_hash_table_lookup(targets_table, (gchar*)mapping->target);
	gchar *target_key = find_target_key(target);
	
	/* Generate an edge table key, which consist of Nix store component:targetProperty */
	gchar *mapping_key = compose_mapping_key(mapping, target_key);
	
	/* Retrieve the dependency array of the mapping from the edges table */
	GPtrArray *dependency_array = g_hash_table_lookup(edges_table, mapping_key);
	
	/* If the dependency array does not exists, create one and add it to the table */
	if(dependency_array == NULL)
	{
	    unsigned int j;
	    GPtrArray *inter_dependencies;

            if(ordering)
	        inter_dependencies = service->depends_on;
	    else
	        inter_dependencies = service->connects_to;

	    /* Create new dependency array */
	    dependency_array = g_ptr_array_new();
	    
	    /* Create a list of mapping values for each dependency */
	    for(j = 0; j < inter_dependencies->len; j++)
	    {
		ServiceMapping *actual_mapping;
		gchar *mapping_value, *target_key;
		Target *target;
		
		/* Retrieve current dependency from the array */
		InterDependencyMapping *dependency = g_ptr_array_index(inter_dependencies, j);
		
		/* Find the activation mapping in the activation array */
		actual_mapping = find_service_mapping(service_mapping_array, dependency);
		
		/* Get the target interface */
		target = g_hash_table_lookup(targets_table, (gchar*)actual_mapping->target);
		target_key = find_target_key(target);
		
		/* Generate mapping value from the service key and target property */
		mapping_value = compose_mapping_key(actual_mapping, target_key);
		
		/* Add mapping value to the dependency array */
		g_ptr_array_add(dependency_array, mapping_value);
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
    g_hash_table_destroy(edges_table);
}

void print_edges_table(GHashTable *edges_table, int ordering)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, edges_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        unsigned int i;
        GPtrArray *dependency_array = (GPtrArray*)value;

        for(i = 0; i < dependency_array->len; i++)
        {
            gchar *dep = g_ptr_array_index(dependency_array, i);
            g_print("\"%s\" -> \"%s\"", (gchar*)key, dep);

            if(!ordering)
                g_print(" [style=dashed]");
            g_print("\n");
        }
    }
}
