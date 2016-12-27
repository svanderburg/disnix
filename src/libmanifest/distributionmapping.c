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

#include "distributionmapping.h"
#include <xmlutil.h>

GPtrArray *generate_distribution_array(const gchar *manifest_file)
{
    /* Declarations */
    xmlDocPtr doc;
    xmlNodePtr node_root;
    xmlXPathObjectPtr result;
    GPtrArray *distribution_array = NULL;
    
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
        g_printerr("The manifest XML file is empty!\n");
	xmlFreeDoc(doc);
	xmlCleanupParser();
	return NULL;
    }
    
    /* Query the distribution elements */
    result = executeXPathQuery(doc, "/manifest/distribution/mapping");
    
    /* Iterate over all the distribution elements and add them to the array */
    
    if(result)
    {
	unsigned int i;
	xmlNodeSetPtr nodeset = result->nodesetval;
	
	/* Create a distribution array */
	distribution_array = g_ptr_array_new();
    
	/* Iterate over all the mapping elements */
	for(i = 0; i < nodeset->nodeNr; i++)
        {
	    xmlNodePtr mapping_children = nodeset->nodeTab[i]->children;
	    DistributionItem *item = (DistributionItem*)g_malloc(sizeof(DistributionItem));
	    gchar *profile = NULL, *target = NULL;
	    
	    /* Iterate over all the mapping item children (profile and target elements) */
	    
	    while(mapping_children != NULL)
	    {
		if(xmlStrcmp(mapping_children->name, (xmlChar*) "profile") == 0)
		    profile = g_strdup((gchar*)mapping_children->children->content);
		else if(xmlStrcmp(mapping_children->name, (xmlChar*) "target") == 0)
		    target = g_strdup((gchar*)mapping_children->children->content);
		
		mapping_children = mapping_children->next;
	    }
	    
	    /* Add the mapping to the array */
	    item->profile = profile;
	    item->target = target;
	    g_ptr_array_add(distribution_array, item);
        }
	
	xmlXPathFreeObject(result);
    }
    
    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Return the distribution array */
    return distribution_array;
}

void delete_distribution_array(GPtrArray *distribution_array)
{
    if(distribution_array != NULL)
    {
        unsigned int i;
    
        for(i = 0; i < distribution_array->len; i++)
        {
            DistributionItem* item = g_ptr_array_index(distribution_array, i);
    
            g_free(item->profile);
            g_free(item->target);
            g_free(item);
        }
    
        g_ptr_array_free(distribution_array, TRUE);
    }
}

static int has_next_distribution_item(void *data)
{
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)data;
    return distribution_iterator_data->index < distribution_iterator_data->length;
}

static pid_t next_distribution_process(void *data)
{
    /* Declarations */
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)data;
    
    /* Retrieve distributionitem, target pair */
    DistributionItem *item = g_ptr_array_index(distribution_iterator_data->distribution_array, distribution_iterator_data->index);
    Target *target = find_target(distribution_iterator_data->target_array, item->target);
    
    /* Invoke the next distribution item operation process */
    pid_t pid = distribution_iterator_data->map_distribution_item(distribution_iterator_data->data, item, target);
    
    /* Increase the iterator index */
    distribution_iterator_data->index++;
    
    if(pid > 0)
    {
        /* Add pid to the pid table so that we know what the corresponding distribution item is */
        gint *pid_ptr = g_malloc(sizeof(gint));
        *pid_ptr = pid;
        g_hash_table_insert(distribution_iterator_data->pid_table, pid_ptr, item);
    }
    
    /* Return the pid of the invoked process */
    return pid;
}

static void complete_distribution_process(void *data, pid_t pid, ProcReact_Status status, int result)
{
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)data;
    
    /* Retrieve corresponding distribution item of the pid */
    gint *pid_ptr = &pid;
    DistributionItem *item = g_hash_table_lookup(distribution_iterator_data->pid_table, pid_ptr);
    
    /* If anything failed set the overall success status to FALSE */
    if(status != PROCREACT_STATUS_OK || !result)
        distribution_iterator_data->success = FALSE;
    
    /* Invoke callback that handles completion of distribution item */
    distribution_iterator_data->complete_distribution_item_mapping(distribution_iterator_data->data, item, status, result);
}

ProcReact_PidIterator create_distribution_iterator(const GPtrArray *distribution_array, const GPtrArray *target_array, map_distribution_item_function map_distribution_item, complete_distribution_item_mapping_function complete_distribution_item_mapping, void *data)
{
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)g_malloc(sizeof(DistributionIteratorData));
    
    distribution_iterator_data->index = 0;
    distribution_iterator_data->length = distribution_array->len;
    distribution_iterator_data->success = TRUE;
    distribution_iterator_data->distribution_array = distribution_array;
    distribution_iterator_data->target_array = target_array;
    distribution_iterator_data->pid_table = g_hash_table_new_full(g_int_hash, g_int_equal, g_free, NULL);
    distribution_iterator_data->map_distribution_item = map_distribution_item;
    distribution_iterator_data->complete_distribution_item_mapping = complete_distribution_item_mapping;
    distribution_iterator_data->data = data;
    
    return procreact_initialize_pid_iterator(has_next_distribution_item, next_distribution_process, procreact_retrieve_boolean, complete_distribution_process, distribution_iterator_data);
}

void destroy_distribution_iterator(ProcReact_PidIterator *iterator)
{
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)iterator->data;
    g_hash_table_destroy(distribution_iterator_data->pid_table);
    g_free(distribution_iterator_data);
}

int distribution_iterator_has_succeeded(const ProcReact_PidIterator *iterator)
{
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)iterator->data;
    return distribution_iterator_data->success;
}
