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

#include "graph.h"
#include "edgestable.h"
#include "clustertable.h"
#include <manifest.h>

int generate_graph(const gchar *manifest_file)
{
    /* Creates a manifest object */
    Manifest *manifest = create_manifest(manifest_file);
    
    if(manifest == NULL)
    {
        g_printerr("Error with opening manifest file!\n");
        return 1;
    }
    else
    {
        /* Creates a table which maps each target onto a list of mappings */
        GHashTable *cluster_table = generate_cluster_table(manifest->activation_array, manifest->target_array);
    
        /* Creates a table which associates each mapping to its dependencies */
        GHashTable *edges_table = generate_edges_table(manifest->activation_array, manifest->target_array);
    
        GHashTableIter iter;
        gpointer *key;
        gpointer *value;
        int count = 0;

        g_print("digraph G {\n");
        
        /* Generate clusters with nodes from the cluster table */
        
        g_hash_table_iter_init(&iter, cluster_table);
        while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&value)) 
        {
            unsigned int i;
            GArray *cluster_array = (GArray*)value;
            gchar *target = (gchar*)key;
        
            g_print("subgraph cluster_%d {\n", count);
            g_print("style=filled;\n");
            g_print("node [style=filled,fillcolor=white,color=black];\n");
        
            for(i = 0; i < cluster_array->len; i++)
            {
                gchar *key = g_array_index(cluster_array, gchar*, i);
                g_print("\"%s:%s\" [ label = \"%s\" ];\n", key, target, key /*TODO: display names service+44*/);
            }
        
            g_print("label = \"%s\"\n", target);
            g_print("}\n");
        
            count++;
        }
    
        /* Generate edges from the edges table */
        
        g_hash_table_iter_init(&iter, edges_table);
        while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&value))
        {
            unsigned int i;
            GArray *dependency_array = (GArray*)value;
        
            for(i = 0; i < dependency_array->len; i++)
            {
                gchar *dep = g_array_index(dependency_array, gchar*, i);
                g_print("\"%s\" -> \"%s\"\n", (gchar*)key, dep);
            }
        }
    
        g_print("}\n");
    
        /* Cleanup */
        destroy_cluster_table(cluster_table);
        destroy_edges_table(edges_table);
        delete_manifest(manifest);
        
        return 0;
    }
}
