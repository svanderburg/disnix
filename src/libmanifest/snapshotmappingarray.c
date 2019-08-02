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

#include "snapshotmappingarray.h"
#include <stdlib.h>
#include <libxml/parser.h>
#include <nixxml-ghashtable.h>
#include <nixxml-gptrarray.h>
#include "manifest.h"

static GPtrArray *filter_selected_mappings(GPtrArray *snapshot_mapping_array, const gchar *container_filter, const gchar *component_filter)
{
    if(component_filter == NULL && container_filter == NULL)
        return snapshot_mapping_array;
    else
    {
        GPtrArray *filtered_snapshot_mapping_array = g_ptr_array_new();
        unsigned int i;

        for(i = 0; i < snapshot_mapping_array->len; i++)
        {
            SnapshotMapping *mapping = g_ptr_array_index(snapshot_mapping_array, i);
            if(mapping_is_selected(mapping, container_filter, component_filter))
                g_ptr_array_add(filtered_snapshot_mapping_array, mapping);
            else
                delete_snapshot_mapping(mapping);
        }

        g_ptr_array_free(snapshot_mapping_array, TRUE);
        return filtered_snapshot_mapping_array;
    }
}

GPtrArray *parse_snapshot_mapping_array(xmlNodePtr element, const gchar *container_filter, const gchar *component_filter, void *userdata)
{
    GPtrArray *snapshot_mapping_array = NixXML_parse_g_ptr_array(element, "mapping", userdata, parse_snapshot_mapping);

    /* Sort the snapshots array */
    g_ptr_array_sort(snapshot_mapping_array, (GCompareFunc)compare_snapshot_mapping);

    /* Filter only selected mappings */
    snapshot_mapping_array = filter_selected_mappings(snapshot_mapping_array, container_filter, component_filter);

    return snapshot_mapping_array;
}

void delete_snapshot_mapping_array(GPtrArray *snapshot_mapping_array)
{
    NixXML_delete_g_ptr_array(snapshot_mapping_array, (NixXML_DeleteGPtrArrayElementFunc)delete_snapshot_mapping);
}

int check_snapshot_mapping_array(const GPtrArray *snapshot_mapping_array)
{
    return NixXML_check_g_ptr_array(snapshot_mapping_array, (NixXML_CheckGPtrArrayElementFunc)check_snapshot_mapping);
}

int compare_snapshot_mapping_arrays(const GPtrArray *snapshot_mapping_array1, const GPtrArray *snapshot_mapping_array2)
{
    if(snapshot_mapping_array1->len == snapshot_mapping_array2->len)
    {
        unsigned int i;

        for(i = 0; i < snapshot_mapping_array1->len; i++)
        {
            SnapshotMapping *mapping = g_ptr_array_index(snapshot_mapping_array1, i);
            if(find_snapshot_mapping(snapshot_mapping_array2, (SnapshotMappingKey*)mapping) == NULL)
                return FALSE;
        }

        return TRUE;
    }
    else
        return FALSE;
}

void print_snapshot_mapping_array_nix(FILE *file, const GPtrArray *snapshot_mapping_array, const int indent_level, void *userdata)
{
    NixXML_print_g_ptr_array_nix(file, snapshot_mapping_array, indent_level, userdata, (NixXML_PrintValueFunc)print_snapshot_mapping_nix);
}

void print_snapshot_mapping_array_xml(FILE *file, const GPtrArray *snapshot_mapping_array, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_g_ptr_array_xml(file, snapshot_mapping_array, "mapping", indent_level, type_property_name, userdata, (NixXML_PrintXMLValueFunc)print_snapshot_mapping_xml);
}

SnapshotMapping *find_snapshot_mapping(const GPtrArray *snapshot_mapping_array, const SnapshotMappingKey *key)
{
    SnapshotMapping **ret = bsearch(&key, snapshot_mapping_array->pdata, snapshot_mapping_array->len, sizeof(gpointer), (int (*)(const void*, const void*)) compare_snapshot_mapping);

    if(ret == NULL)
        return NULL;
    else
        return *ret;
}

GPtrArray *subtract_snapshot_mappings(const GPtrArray *snapshot_mapping_array1, const GPtrArray *snapshot_mapping_array2)
{
    GPtrArray *return_array = g_ptr_array_new();
    unsigned int i;

    for(i = 0; i < snapshot_mapping_array1->len; i++)
    {
        SnapshotMapping *mapping = g_ptr_array_index(snapshot_mapping_array1, i);

        if(find_snapshot_mapping(snapshot_mapping_array2, (SnapshotMappingKey*)mapping) == NULL)
            g_ptr_array_add(return_array, mapping);
    }

    return return_array;
}

GPtrArray *find_snapshot_mappings_per_target(const GPtrArray *snapshot_mapping_array, const gchar *target)
{
    GPtrArray *return_array = g_ptr_array_new();
    unsigned int i;

    for(i = 0; i < snapshot_mapping_array->len; i++)
    {
        SnapshotMapping *mapping = g_ptr_array_index(snapshot_mapping_array, i);

        if(xmlStrcmp(mapping->target, (const xmlChar*) target) == 0)
            g_ptr_array_add(return_array, mapping);
    }

    return return_array;
}

void reset_snapshot_items_transferred_status(GPtrArray *snapshot_mapping_array)
{
    unsigned int i;

    for(i = 0; i < snapshot_mapping_array->len; i++)
    {
        SnapshotMapping *mapping = g_ptr_array_index(snapshot_mapping_array, i);
        mapping->transferred = FALSE;
    }
}
