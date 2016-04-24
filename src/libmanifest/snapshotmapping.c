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

#include "snapshotmapping.h"
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <xmlutil.h>
#define min(a,b) ((a) < (b) ? (a) : (b))

static gint compare_snapshot_mapping_keys(const SnapshotMappingKey **l, const SnapshotMappingKey **r)
{
    const SnapshotMappingKey *left = *l;
    const SnapshotMappingKey *right = *r;
    
    /* Compare the component names */
    gint status = g_strcmp0(left->component, right->component);
    
    if(status == 0)
    {
        gint status = g_strcmp0(left->target, right->target); /* If components are equal then compare the targets */
        
        if(status == 0)
            return g_strcmp0(left->container, right->container); /* If containers are equal then compare the containers */
        else
            return status;
    }
    else
        return status;
}

static gint compare_snapshot_mapping(const SnapshotMapping **l, const SnapshotMapping **r)
{
    return compare_snapshot_mapping_keys((const SnapshotMappingKey **)l, (const SnapshotMappingKey **)r);
}

GPtrArray *create_snapshots_array(const gchar *manifest_file)
{
    xmlDocPtr doc;
    xmlNodePtr node_root;
    xmlXPathObjectPtr result;
    GPtrArray *snapshots_array;
    
    /* Parse the XML document */
    
    if((doc = xmlParseFile(manifest_file)) == NULL)
    {
	g_printerr("Error with parsing the manifest XML file!\n");
	xmlCleanupParser();
	return NULL;
    }

    /* Retrieve root element */
    node_root = xmlDocGetRootElement(doc);
    
    if(node_root == NULL)
    {
        g_printerr("The distribution manifest XML file is empty!\n");
	xmlFreeDoc(doc);
	xmlCleanupParser();
	return NULL;
    }

    /* Query the distribution elements */
    result = executeXPathQuery(doc, "/manifest/snapshots/mapping");
    
    /* Initialize snapshots array */
    snapshots_array = g_ptr_array_new();
    
    /* Iterate over all the distribution elements and add them to the array */
    
    if(result)
    {
	xmlNodeSetPtr nodeset = result->nodesetval;
	unsigned int i;
	
	/* Iterate over all the mapping elements */
	for(i = 0; i < nodeset->nodeNr; i++)
        {
	    xmlNodePtr mapping_children = nodeset->nodeTab[i]->children;
	    gchar *component = NULL;
	    gchar *container = NULL;
	    gchar *target = NULL;
	    gchar *service = NULL;
	    gchar *type = NULL;
	    SnapshotMapping *mapping = (SnapshotMapping*)g_malloc(sizeof(SnapshotMapping));
	    
	    /* Iterate over all the mapping item children (service,target,targetProperty,type,dependsOn elements) */
	    
	    while(mapping_children != NULL)
	    {
		if(xmlStrcmp(mapping_children->name, (xmlChar*) "component") == 0)
		    component = g_strdup((gchar*)mapping_children->children->content);
		else if(xmlStrcmp(mapping_children->name, (xmlChar*) "container") == 0)
		    container = g_strdup((gchar*)mapping_children->children->content);
		else if(xmlStrcmp(mapping_children->name, (xmlChar*) "target") == 0)
		    target = g_strdup((gchar*)mapping_children->children->content);
		else if(xmlStrcmp(mapping_children->name, (xmlChar*) "service") == 0)
		    service = g_strdup((gchar*)mapping_children->children->content);
		else if(xmlStrcmp(mapping_children->name, (xmlChar*) "type") == 0)
		    type = g_strdup((gchar*)mapping_children->children->content);
		
		mapping_children = mapping_children->next;
	    }
	    mapping->component = component;
	    mapping->container = container;
	    mapping->target = target;
	    mapping->service = service;
	    mapping->type = type;
	    mapping->transferred = FALSE;
	    
	    /* Add the mapping to the array */
	    g_ptr_array_add(snapshots_array, mapping);
	}
    }

    /* Cleanup */
    xmlXPathFreeObject(result);
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Sort the snapshots array */
    g_ptr_array_sort(snapshots_array, (GCompareFunc)compare_snapshot_mapping);
    
    /* Return the snapshots array */
    return snapshots_array;
}

void delete_snapshots_array(GPtrArray *snapshots_array)
{
    if(snapshots_array != NULL)
    {
        unsigned int i;
    
        for(i = 0; i < snapshots_array->len; i++)
        {
            SnapshotMapping *mapping = g_ptr_array_index(snapshots_array, i);
            g_free(mapping->component);
            g_free(mapping->container);
            g_free(mapping->target);
            g_free(mapping->service);
            g_free(mapping->type);
            g_free(mapping);
        }
    
        g_ptr_array_free(snapshots_array, TRUE);
    }
}

SnapshotMapping *find_snapshot_mapping(const GPtrArray *snapshots_array, const SnapshotMappingKey *key)
{
    SnapshotMapping **ret = bsearch(&key, snapshots_array->pdata, snapshots_array->len, sizeof(gpointer), (int (*)(const void*, const void*)) compare_snapshot_mapping);
    
    if(ret == NULL)
        return NULL;
    else
        return *ret;
}

GPtrArray *subtract_snapshot_mappings(GPtrArray *snapshots_array1, GPtrArray *snapshots_array2)
{
    GPtrArray *return_array = g_ptr_array_new();
    unsigned int i;
    
    for(i = 0; i < snapshots_array1->len; i++)
    {
        SnapshotMapping *mapping = g_ptr_array_index(snapshots_array1, i);
        
        if(find_snapshot_mapping(snapshots_array2, (SnapshotMappingKey*)mapping) == NULL)
            g_ptr_array_add(return_array, mapping);
    }
    
    return return_array;
}

GPtrArray *find_snapshot_mappings_per_target(const GPtrArray *snapshots_array, const gchar *target)
{
    GPtrArray *return_array = g_ptr_array_new();
    unsigned int i;
    
    for(i = 0; i < snapshots_array->len; i++)
    {
        SnapshotMapping *mapping = g_ptr_array_index(snapshots_array, i);
        
        if(g_strcmp0(mapping->target, target) == 0)
            g_ptr_array_add(return_array, mapping);
    }
    
    return return_array;
}
