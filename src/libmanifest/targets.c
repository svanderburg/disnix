/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2011  Sander van der Burg
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

#include "targets.h"
#include <xmlutil.h>

GArray *generate_target_array(const gchar *manifest_file)
{
    /* Declarations */
    xmlDocPtr doc;
    xmlNodePtr node_root;
    xmlXPathObjectPtr result;
    GArray *targets_array = NULL;
    
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
    
    /* Query the targets elements */
    result = executeXPathQuery(doc, "/manifest/targets/target");
    
    /* Iterate over all the targets elements and add them to the array */
    
    if(result)
    {
	unsigned int i;
	xmlNodeSetPtr nodeset = result->nodesetval;
	
	/* Create a targets array */
        targets_array = g_array_new(FALSE, FALSE, sizeof(gchar*));
	
	/* Iterate over all the mapping elements */
	for(i = 0; i < nodeset->nodeNr; i++)
        {
	    xmlNodePtr targets_children = nodeset->nodeTab[i]->children;
	    gchar *target = g_strdup(targets_children->content);
	    
	    /* Add target to the targets array */
	    g_array_append_val(targets_array, target);
	}
	
	xmlXPathFreeObject(result);
    }
    
    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Sort the array */
    g_array_sort(targets_array, (GCompareFunc)g_strcmp0);
    
    /* Return the distribution array */
    return targets_array;
}

void delete_target_array(GArray *target_array)
{
    unsigned int i;
    
    for(i = 0; i < target_array->len; i++)
    {
	gchar *target = g_array_index(target_array, gchar*, i);
	g_free(target);
    }
    
    g_array_free(target_array, TRUE);
}

int target_index(const GArray *target_array, const gchar *target)
{
    gint left = 0;
    gint right = target_array->len - 1;
    
    while(left <= right)
    {
	gint mid = (left + right) / 2;
	gchar *mid_target = g_array_index(target_array, gchar*, mid);
        gint status = g_strcmp0(mid_target, target);
	
	if(status == 0)
            return mid; /* Return index of the found target */
	else if(status > 0)
	    right = mid - 1;
	else if(status < 0)
	    left = mid + 1;
    }
    
    return -1; /* Target not found */
}
