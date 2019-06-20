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

#include "graph.h"
#include "edgestable.h"
#include "clustertable.h"
#include <manifest.h>
#include <activationmapping.h>

int generate_graph(const gchar *manifest_file, const gchar *coordinator_profile_path, gchar *profile, int no_containers)
{
    Manifest *manifest = open_provided_or_previous_manifest_file(manifest_file, coordinator_profile_path, profile, MANIFEST_ACTIVATION_FLAG | MANIFEST_TARGETS_FLAG, NULL, NULL);

    if(manifest == NULL)
    {
        g_printerr("Cannot open any manifest file!\n");
        g_printerr("Please provide a valid manifest as command-line parameter!\n");
        return 1;
    }
    else
    {
        int exit_status;

        if(check_manifest(manifest))
        {
            /* Creates a table which maps each target onto a list of mappings */
            GHashTable *cluster_table = generate_cluster_table(manifest->activation_array, manifest->targets_table);

            /* Creates a table which associates each mapping to its inter-dependencies that have a strict ordering requirement */
            GHashTable *edges_depends_on_table = generate_edges_table(manifest->activation_array, manifest->targets_table, TRUE);

            /* Creates a table which associates each mapping to its inter-dependencies that have no strict ordering requirement */
            GHashTable *edges_connects_to_table = generate_edges_table(manifest->activation_array, manifest->targets_table, FALSE);

            g_print("digraph G {\n");

            print_cluster_table(cluster_table, no_containers); /* Generate clusters with nodes from the cluster table */

            /* Generate edges from the edges table */
            print_edges_table(edges_depends_on_table, TRUE);
            print_edges_table(edges_connects_to_table, FALSE);

            g_print("}\n");

            /* Cleanup */
            destroy_cluster_table(cluster_table);
            destroy_edges_table(edges_connects_to_table);
            destroy_edges_table(edges_depends_on_table);

            exit_status = 0;
        }
        else
            exit_status = 1;

        delete_manifest(manifest);

        return exit_status;
    }
}
