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

#include "interfaces.h"
#include <stdlib.h>
#include <xmlutil.h>

static gint compare_interface(const Interface **l, const Interface **r)
{
    const Interface *left = *l;
    const Interface *right = *r;
    
    return g_strcmp0(left->target, right->target);
}

static int compare_interface_keys(const char *key, const Interface **r)
{
    const Interface *right = *r;
    return g_strcmp0(key, right->target);
}

GPtrArray *create_interface_array(const gchar *distributed_derivation_file)
{
    /* Declarations */
    xmlDocPtr doc;
    xmlNodePtr node_root;
    xmlXPathObjectPtr result;
    GPtrArray *interface_array = NULL;
    
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
    
    /* Query the targets elements */
    result = executeXPathQuery(doc, "/distributedderivation/interfaces/interface");
    
    /* Iterate over all the targets elements and add them to the array */
    
    if(result)
    {
	unsigned int i;
	xmlNodeSetPtr nodeset = result->nodesetval;
	
	/* Create a targets array */
	interface_array = g_ptr_array_new();
	
	/* Iterate over all the target elements */
	for(i = 0; i < nodeset->nodeNr; i++)
	{
	    xmlNodePtr interfaces_children = nodeset->nodeTab[i]->children;
	    gchar *target = NULL;
	    gchar *clientInterface = NULL;
	    Interface *interface = (Interface*)g_malloc(sizeof(Interface));
	    
	    while(interfaces_children != NULL)
	    {
	        if(xmlStrcmp(interfaces_children->name, (xmlChar*) "target") == 0)
		    target = g_strdup((gchar*)interfaces_children->children->content);
		else if(xmlStrcmp(interfaces_children->name, (xmlChar*) "clientInterface") == 0)
		    clientInterface = g_strdup((gchar*)interfaces_children->children->content);
	        
	        interfaces_children = interfaces_children->next;
	    }
	    
	    interface->target = target;
	    interface->clientInterface = clientInterface;
	    
	    /* Add interface item to the interface array */
	    g_ptr_array_add(interface_array, interface);
	}
	
	/* Sort the interface array */
	g_ptr_array_sort(interface_array, (GCompareFunc)compare_interface);
	
	/* Cleanup */
	xmlXPathFreeObject(result);
    }
    
    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();
    
    /* Return the interfaces array */
    return interface_array;
}

void delete_interface_array(GPtrArray *interface_array)
{
    if(interface_array != NULL)
    {
        unsigned int i;
        
        for(i = 0; i < interface_array->len; i++)
        {
            Interface *interface = g_ptr_array_index(interface_array, i);
            g_free(interface);
        }
    
        g_ptr_array_free(interface_array, TRUE);
    }
}

Interface *find_interface(const GPtrArray *interface_array, const gchar *key)
{
    Interface **ret = bsearch(key, interface_array->pdata, interface_array->len, sizeof(gpointer), (int (*)(const void *, const void *)) compare_interface_keys);
    
    if(ret == NULL)
        return NULL;
    else
        return *ret;
}
