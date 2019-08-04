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

#include "capture-infra.h"
#include <stdlib.h>
#include <targets-iterator.h>
#include <client-interface.h>
#include "nixxml-ghashtable-iter.h"

static ProcReact_Future capture_infra_on_target(void *data, Target *target, gchar *client_interface, gchar *target_key)
{
    return exec_capture_config(client_interface, target_key);
}

static void complete_capture_infra_on_target(void *data, Target *target, gchar *target_key, ProcReact_Future *future, ProcReact_Status status)
{
    GHashTable *configs_table = (GHashTable*)data;

    if(status != PROCREACT_STATUS_OK || future->result == NULL)
        g_printerr("[target: %s]: Cannot capture the infrastructure!\n", target_key);
    else
        g_hash_table_insert(configs_table, target_key, future->result);
}

static void print_configs_table(GHashTable *configs_table)
{
    NixXML_GHashTableOrderedIter iter;
    gchar *key;
    gpointer value;

    /* Print the captured configs per machine in deterministic order */
    g_print("{\n");

    NixXML_g_hash_table_ordered_iter_init(&iter, configs_table);
    while(NixXML_g_hash_table_ordered_iter_next(&iter, &key, &value))
    {
        char **config = (char**)value;
        char *line;
        unsigned int count = 0;

        g_print("  \"%s\" = ", key);

        /* Iterate over each line of the captured config */
        while((line = config[count]) != NULL)
        {
            g_print("%s", line);
            count++;

            if(config[count] != NULL)
                g_print("\n  ");
        }

        g_print(";\n");
    }

    g_print("}\n");

    NixXML_g_hash_table_ordered_iter_destroy(&iter);
}

static void delete_configs_table(GHashTable *configs_table)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, configs_table);
    while (g_hash_table_iter_next(&iter, &key, &value))
        procreact_free_string_array((char**)value);

    g_hash_table_destroy(configs_table);
}

int capture_infra(gchar *interface, gchar *target_property, gchar *infrastructure_expr, const int xml)
{
    /* Retrieve an array of all target machines from the infrastructure expression */
    GHashTable *targets_table = create_targets_table(infrastructure_expr, xml, target_property, interface);

    if(targets_table == NULL)
    {
        g_printerr("[coordinator]: Error retrieving targets from infrastructure model!\n");
        return 1;
    }
    else
    {
        int exit_status;

        if(check_targets_table(targets_table))
        {
            GHashTable *configs_table = g_hash_table_new(g_str_hash, g_str_equal);

            /* Iterate over targets and capture their infrastructure configurations */
            ProcReact_FutureIterator iterator = create_target_future_iterator(targets_table, target_property, interface, capture_infra_on_target, complete_capture_infra_on_target, configs_table);
            procreact_fork_in_parallel_buffer_and_wait(&iterator);
            exit_status = !target_iterator_has_succeeded(iterator.data);

            /* Print the captured configurations */
            print_configs_table(configs_table);

            /* Cleanup */
            destroy_target_future_iterator(&iterator);
            delete_configs_table(configs_table);
        }
        else
            exit_status = 1;

        delete_targets_table(targets_table);

        /* Return exit status */
        return exit_status;
    }
}
