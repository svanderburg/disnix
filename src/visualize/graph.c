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

#include "graph.h"
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include "edgestable.h"
#include "clustertable.h"
#include <manifest.h>
#include <activationmapping.h>

int generate_graph(const gchar *manifest_file, const gchar *coordinator_profile_path, gchar *profile, int no_containers)
{
    Manifest *manifest;
    
    if(manifest_file == NULL)
    {
        /* Get current username */
        char *username = (getpwuid(geteuid()))->pw_name;
        
        /* If no manifest file has been provided, try opening the last deployed one */
        gchar *old_manifest_file = determine_previous_manifest_file(coordinator_profile_path, username, profile);
        
        if(old_manifest_file == NULL)
        {
            g_printerr("[coordinator]: No previous manifest file exists, so no visualization will be provided!\n");
            return 0;
        }
        else
        {
            manifest = create_manifest(old_manifest_file);
            g_free(old_manifest_file);
        }
    }
    else
        manifest = create_manifest(manifest_file); /* Open the provided manifest */

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
            gchar *target = (gchar*)key;
            GHashTable *container_table = (GHashTable*)value;
            GHashTableIter iter2;
            gpointer *key2;
            gpointer *value2;
            int count2 = 0;
            
            g_print("subgraph cluster_%d {\n", count);
            g_print("style=filled;\n");
            g_print("label=\"%s\";\n", target);
            g_print("fillcolor=grey\n");

            g_hash_table_iter_init(&iter2, container_table);
            
            while(g_hash_table_iter_next(&iter2, (gpointer*)&key2, (gpointer*)&value2))
            {
                unsigned int i;
                gchar *container_key = (gchar*)key2;
                GPtrArray *cluster_array = (GPtrArray*)value2;
                
                if(!no_containers)
                {
                    g_print("subgraph cluster_%d_%d {\n", count, count2);
                    g_print("style=filled;\n");
                
                    if(cluster_array->len > 0)
                    {
                        ActivationMapping *mapping = g_ptr_array_index(cluster_array, 0);
                        g_print("label=\"%s\";\n", mapping->container);
                    }
                    
                    g_print("fillcolor=grey40\n");
                }
                
                g_print("node [style=filled,fillcolor=white,color=black];\n");
                
                for(i = 0; i < cluster_array->len; i++)
                {
                    ActivationMapping *mapping = g_ptr_array_index(cluster_array, i);
                    g_print("\"%s:%s:%s\" [ label = \"%s\" ];\n", mapping->key, target, mapping->container, mapping->name);
                }
                
                if(!no_containers)
                    g_print("}\n");
                
                count2++;
            }
            
            g_print("}\n");
        
            count++;
        }
    
        /* Generate edges from the edges table */
        
        g_hash_table_iter_init(&iter, edges_table);
        while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&value))
        {
            unsigned int i;
            GPtrArray *dependency_array = (GPtrArray*)value;
        
            for(i = 0; i < dependency_array->len; i++)
            {
                gchar *dep = g_ptr_array_index(dependency_array, i);
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
