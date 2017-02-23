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
    GPtrArray *distribution_array;
    
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

    /* Create a distribution array */
    distribution_array = g_ptr_array_new();
    
    /* Query the distribution elements */
    result = executeXPathQuery(doc, "/manifest/distribution/mapping");
    
    /* Iterate over all the distribution elements and add them to the array */
    
    if(result)
    {
	unsigned int i;
	xmlNodeSetPtr nodeset = result->nodesetval;
	
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
		    profile = duplicate_node_text(mapping_children);
		else if(xmlStrcmp(mapping_children->name, (xmlChar*) "target") == 0)
		    target = duplicate_node_text(mapping_children);
		
		mapping_children = mapping_children->next;
	    }
	    
	    item->profile = profile;
	    item->target = target;
	    
	    if(item->profile == NULL || item->target == NULL)
	    {
	        /* Check if all mandatory properties have been provided */
	        g_printerr("A mandatory property seems to be missing. Have you provided a correct\n");
	        g_printerr("manifest file?\n");
	        delete_distribution_array(distribution_array);
	        distribution_array = NULL;
	        break;
	    }
	    else
	        g_ptr_array_add(distribution_array, item); /* Add the mapping to the array */
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
    return has_next_iteration_process(&distribution_iterator_data->model_iterator_data);
}

static pid_t next_distribution_process(void *data)
{
    /* Declarations */
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)data;
    
    /* Retrieve distributionitem, target pair */
    DistributionItem *item = g_ptr_array_index(distribution_iterator_data->distribution_array, distribution_iterator_data->model_iterator_data.index);
    Target *target = find_target(distribution_iterator_data->target_array, item->target);
    
    /* Invoke the next distribution item operation process */
    pid_t pid = distribution_iterator_data->map_distribution_item(distribution_iterator_data->data, item, target);
    
    /* Increase the iterator index and update the pid table */
    next_iteration_process(&distribution_iterator_data->model_iterator_data, pid, item);
    
    /* Return the pid of the invoked process */
    return pid;
}

static void complete_distribution_process(void *data, pid_t pid, ProcReact_Status status, int result)
{
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)data;
    
    /* Retrieve the completed item */
    DistributionItem *item = complete_iteration_process(&distribution_iterator_data->model_iterator_data, pid, status, result);
    
    /* Invoke callback that handles completion of distribution item */
    distribution_iterator_data->complete_distribution_item_mapping(distribution_iterator_data->data, item, status, result);
}

ProcReact_PidIterator create_distribution_iterator(const GPtrArray *distribution_array, const GPtrArray *target_array, map_distribution_item_function map_distribution_item, complete_distribution_item_mapping_function complete_distribution_item_mapping, void *data)
{
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)g_malloc(sizeof(DistributionIteratorData));
    
    init_model_iterator_data(&distribution_iterator_data->model_iterator_data, distribution_array->len);
    distribution_iterator_data->distribution_array = distribution_array;
    distribution_iterator_data->target_array = target_array;
    distribution_iterator_data->map_distribution_item = map_distribution_item;
    distribution_iterator_data->complete_distribution_item_mapping = complete_distribution_item_mapping;
    distribution_iterator_data->data = data;
    
    return procreact_initialize_pid_iterator(has_next_distribution_item, next_distribution_process, procreact_retrieve_boolean, complete_distribution_process, distribution_iterator_data);
}

void destroy_distribution_iterator(ProcReact_PidIterator *iterator)
{
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)iterator->data;
    destroy_model_iterator_data(&distribution_iterator_data->model_iterator_data);
    g_free(distribution_iterator_data);
}

int distribution_iterator_has_succeeded(const ProcReact_PidIterator *iterator)
{
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)iterator->data;
    return distribution_iterator_data->model_iterator_data.success;
}
