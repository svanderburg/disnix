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

#include "oldsnapshotmapping.h"
#include <stdlib.h>
#include <libxml/parser.h>
#include <nixxml-gptrarray.h>

static gint compare_snapshot_mapping_keys(const OldSnapshotMappingKey **l, const OldSnapshotMappingKey **r)
{
    const OldSnapshotMappingKey *left = *l;
    const OldSnapshotMappingKey *right = *r;

    /* Compare the component names */
    gint status = xmlStrcmp(left->component, right->component);

    if(status == 0)
    {
        gint status = xmlStrcmp(left->target, right->target); /* If components are equal then compare the targets */

        if(status == 0)
            return xmlStrcmp(left->container, right->container); /* If containers are equal then compare the containers */
        else
            return status;
    }
    else
        return status;
}

static gint compare_snapshot_mapping(const OldSnapshotMapping **l, const OldSnapshotMapping **r)
{
    return compare_snapshot_mapping_keys((const OldSnapshotMappingKey **)l, (const OldSnapshotMappingKey **)r);
}

static void delete_old_snapshot_mapping(OldSnapshotMapping *mapping)
{
    xmlFree(mapping->component);
    xmlFree(mapping->container);
    xmlFree(mapping->target);
    xmlFree(mapping->service);
    xmlFree(mapping->type);
    g_free(mapping);
}

static void *create_snapshot_mapping(xmlNodePtr element, void *userdata)
{
    return g_malloc0(sizeof(OldSnapshotMapping));
}

static void insert_snapshot_mapping_attributes(void *table, const xmlChar *key, void *value, void *userdata)
{
    OldSnapshotMapping *mapping = (OldSnapshotMapping*)table;

    if(xmlStrcmp(key, (xmlChar*) "component") == 0)
        mapping->component = value;
    else if(xmlStrcmp(key, (xmlChar*) "container") == 0)
        mapping->container = value;
    else if(xmlStrcmp(key, (xmlChar*) "target") == 0)
        mapping->target = value;
    else if(xmlStrcmp(key, (xmlChar*) "service") == 0)
        mapping->service = value;
    else if(xmlStrcmp(key, (xmlChar*) "type") == 0)
        mapping->type = value;
    else
        xmlFree(value);
}

static gpointer parse_snapshot_mapping(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_simple_attrset(element, userdata, create_snapshot_mapping, NixXML_parse_value, insert_snapshot_mapping_attributes);
}

GPtrArray *parse_old_snapshots(xmlNodePtr element)
{
    GPtrArray *snapshots_array = NixXML_parse_g_ptr_array(element, "mapping", NULL, parse_snapshot_mapping);

    /* Sort the snapshots array */
    g_ptr_array_sort(snapshots_array, (GCompareFunc)compare_snapshot_mapping);

    return snapshots_array;
}

void delete_old_snapshots_array(GPtrArray *snapshots_array)
{
    if(snapshots_array != NULL)
    {
        unsigned int i;

        for(i = 0; i < snapshots_array->len; i++)
        {
            OldSnapshotMapping *mapping = g_ptr_array_index(snapshots_array, i);
            delete_old_snapshot_mapping(mapping);
        }

        g_ptr_array_free(snapshots_array, TRUE);
    }
}

int check_old_snapshots_array(const GPtrArray *snapshots_array)
{
    if(snapshots_array == NULL)
        return TRUE;
    else
    {
        unsigned int i;

        for(i = 0; i < snapshots_array->len; i++)
        {
            OldSnapshotMapping *mapping = g_ptr_array_index(snapshots_array, i);

            if(mapping->component == NULL || mapping->container == NULL || mapping->target == NULL || mapping->service == NULL || mapping->type == NULL)
            {
                /* Check if all mandatory properties have been provided */
                g_printerr("A mandatory property seems to be missing. Have you provided a correct\n");
                g_printerr("manifest file?\n");
                return FALSE;
            }
        }

        return TRUE;
    }
}
