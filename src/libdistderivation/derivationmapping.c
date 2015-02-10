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

#include "derivationmapping.h"
#include <xmlutil.h>

GPtrArray *create_derivation_array(const gchar *distributed_derivation_file)
{
    /* Declarations */
    xmlDocPtr doc;
    xmlNodePtr node_root;
    xmlXPathObjectPtr result;
    GPtrArray *derivation_array = NULL;
    
    /* Parse the XML document */
    
    if((doc = xmlParseFile(distributed_derivation_file)) == NULL)
    {
	g_printerr("Error with parsing the distributed derivation XML file!\n");
	xmlCleanupParser();
	return NULL;
    }
    
    /* Retrieve root element */
    node_root = xmlDocGetRootElement(doc);
    
    if(node_root == NULL)
    {
        g_printerr("The distributed derivation XML file is empty!\n");
	xmlFreeDoc(doc);
	xmlCleanupParser();
	return NULL;
    }

    /* Query the mapping elements */
    result = executeXPathQuery(doc, "/distributedderivation/mapping");
    
    /* Iterate over all the mapping elements and add them to the array */
    
    if(result)
    {
	xmlNodeSetPtr nodeset = result->nodesetval;
	unsigned int i;
	
	/* Create a derivation array */
        derivation_array = g_ptr_array_new();
	
	/* Iterate over all the mapping elements */
	for(i = 0; i < nodeset->nodeNr; i++)
        {
	    xmlNodePtr mapping_children = nodeset->nodeTab[i]->children;
	    DerivationItem *item = (DerivationItem*)g_malloc(sizeof(DerivationItem));
	    gchar *derivation = NULL, *target = NULL;
	    
	    /* Iterate over all the mapping item children (derivation and target elements) */
	    
	    while(mapping_children != NULL)
	    {
		if(xmlStrcmp(mapping_children->name, (xmlChar*) "derivation") == 0)
		    derivation = g_strdup((gchar*) mapping_children->children->content);
		else if(xmlStrcmp(mapping_children->name, (xmlChar*) "target") == 0)
		    target = g_strdup((gchar*) mapping_children->children->content);
		
		mapping_children = mapping_children->next;
	    }
	    
	    /* Added the mapping to the array */
	    item->derivation = derivation;
	    item->target = target;
	    g_ptr_array_insert(derivation_array, -1, item);
        }
    }
    
    /* Cleanup */
    xmlXPathFreeObject(result);
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Return the derivation array */
    return derivation_array;
}

void delete_derivation_array(GPtrArray *derivation_array)
{
    if(derivation_array != NULL)
    {
        unsigned int i;
    
        for(i = 0; i < derivation_array->len; i++)
        {
            DerivationItem *item = g_ptr_array_index(derivation_array, i);
            free(item->derivation);
            g_free(item->target);
            g_free(item);
        }
    
        g_ptr_array_free(derivation_array, TRUE);
    }
}
