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

#include "manifestservicestable.h"
#include <nixxml-ghashtable.h>
#include "interdependencymapping.h"

GHashTable *parse_services_table(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_g_hash_table_verbose(element, "service", "name", userdata, parse_manifest_service);
}

void delete_services_table(GHashTable *services_table)
{
    NixXML_delete_g_hash_table(services_table, (NixXML_DeleteGHashTableValueFunc)delete_manifest_service);
}

int check_services_table(GHashTable *services_table)
{
    return NixXML_check_g_hash_table(services_table, (NixXML_CheckGHashTableValueFunc)check_manifest_service);
}

int compare_services_tables(GHashTable *services_table1, GHashTable *services_table2)
{
    return NixXML_compare_g_hash_tables(services_table1, services_table2, (NixXML_CompareGHashTableValueFunc)compare_manifest_services);
}

GHashTable *generate_union_services_table(GHashTable *left, GHashTable *right)
{
    GHashTable *result_table = g_hash_table_new(g_str_hash, g_str_equal);
    GHashTableIter iter;
    gpointer key, value;

    /* Insert all elements from left */
    g_hash_table_iter_init(&iter, left);
    while(g_hash_table_iter_next(&iter, &key, &value))
        g_hash_table_insert(result_table, key, value);

    /* Insert all elements from right that have not been inserted yet */
    g_hash_table_iter_init(&iter, right);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        if(!g_hash_table_contains(left, key))
            g_hash_table_insert(result_table, key, value);
    }

    return result_table;
}

void print_services_table_nix(FILE *file, GHashTable *services_table, const int indent_level, void *userdata)
{
    NixXML_print_g_hash_table_ordered_nix(file, services_table, indent_level, userdata, (NixXML_PrintValueFunc)print_manifest_service_nix);
}

void print_services_table_xml(FILE *file, GHashTable *services_table, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_g_hash_table_verbose_ordered_xml(file, services_table, "service", "name", indent_level, NULL, userdata, (NixXML_PrintXMLValueFunc)print_manifest_service_xml);
}

ManifestService *find_container_service_dependency(GHashTable *services_table, const ManifestService *manifest_service, const xmlChar *container, const xmlChar *target)
{
    unsigned int i;

    for(i = 0; i < manifest_service->depends_on->len; i++)
    {
        InterDependencyMapping *dependency_mapping = g_ptr_array_index(manifest_service->depends_on, i);

        if((target == NULL && dependency_mapping->target == NULL) || xmlStrcmp(target, dependency_mapping->target) == 0)
        {
            ManifestService *dependency_service = g_hash_table_lookup(services_table, (gchar*)dependency_mapping->service);
            if(g_hash_table_contains(dependency_service->provides_containers_table, (gchar*)container))
                return dependency_service;
        }
    }

    return NULL;
}
