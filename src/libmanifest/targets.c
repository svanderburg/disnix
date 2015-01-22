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

#include "targets.h"
#include <stdlib.h>
#include <xmlutil.h>

static gint compare_target_property(const TargetProperty **l, const TargetProperty **r)
{
    const TargetProperty *left = *l;
    const TargetProperty *right = *r;
    
    return g_strcmp0(left->name, right->name);
}

static gint compare_target(const GArray **l, const GArray **r)
{
    const GArray *left = *l;
    const GArray *right = *r;
    
    gchar *left_target_property = get_target_key(left);
    gchar *right_target_property = get_target_key(right);
    
    return g_strcmp0(left_target_property, right_target_property);
}

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
	targets_array = g_array_new(FALSE, FALSE, sizeof(GArray*));
	
	/* Iterate over all the target elements */
	for(i = 0; i < nodeset->nodeNr; i++)
	{
	    xmlNodePtr targets_children = nodeset->nodeTab[i]->children;
	    GArray *target = g_array_new(FALSE, FALSE, sizeof(GArray*));
	    
	    while(targets_children != NULL)
	    {
	        TargetProperty *targetProperty = (TargetProperty*)g_malloc(sizeof(TargetProperty));
	        targetProperty->name = g_strdup(targets_children->name);
	        targetProperty->value = g_strdup(targets_children->children->content);
	        
	        g_array_append_val(target, targetProperty);
	        
	        targets_children = targets_children->next;
	    }
	    
	    /* Sort the target properties */
	    g_array_sort(target, (GCompareFunc)compare_target_property);
	    
	    /* Add target item to the targets array */
	    g_array_append_val(targets_array, target);
	}
	
	/* Sort the targets array */
	g_array_sort(targets_array, (GCompareFunc)compare_target);
	
	xmlXPathFreeObject(result);
    }
    
    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();
    
    /* Return the distribution array */
    return targets_array;
}

void delete_target_array(GArray *target_array)
{
    if(target_array != NULL)
    {
        unsigned int i;
        
        for(i = 0; i < target_array->len; i++)
        {
            GArray *target = g_array_index(target_array, GArray*, i);
            unsigned int j;
            
            for(j = 0; j < target->len; j++)
            {
                TargetProperty *targetProperty = g_array_index(target, TargetProperty*, j);
                
                g_free(targetProperty->name);
                g_free(targetProperty->value);
                g_free(targetProperty);
            }
            
            g_array_free(target, TRUE);
        }
    
        g_array_free(target_array, TRUE);
    }
}

void print_target_array(const GArray *target_array)
{
    unsigned int i;
    
    for(i = 0; i < target_array->len; i++)
    {
        GArray *target = g_array_index(target_array, GArray*, i);
        unsigned int j;
        
        g_print("Target:\n");
        
        for(j = 0; j < target->len; j++)
        {
            TargetProperty* targetProperty = g_array_index(target, TargetProperty*, j);
            g_print("  %s = %s;\n", targetProperty->name, targetProperty->value);
        }
        
        g_print("\n");
    }
}

int target_index(const GArray *target_array, const gchar *key)
{
    gint left = 0;
    gint right = target_array->len - 1;
    
    while(left <= right)
    {
	gint mid = (left + right) / 2;
	GArray *mid_target = g_array_index(target_array, GArray*, mid);
	gchar *target_key = get_target_key(mid_target);
	gint status = g_strcmp0(target_key, key);
	
	if(status == 0)
	    return mid; /* Return index of the found target */
	else if(status > 0)
	    right = mid - 1;
	else if(status < 0)
	    left = mid + 1;
    }
    
    return -1; /* Target not found */
}

GArray *get_target(const GArray *target_array, const gchar *key)
{
    int index = target_index(target_array, key);
    
    if(index == -1)
        return NULL;
    else
        return g_array_index(target_array, GArray*, index);
}

int target_property_index(const GArray *target, const gchar *name)
{
    gint left = 0;
    gint right = target->len - 1;
    
    while(left <= right)
    {
	gint mid = (left + right) / 2;
	TargetProperty *mid_target_property = g_array_index(target, TargetProperty*, mid);
	gint status = g_strcmp0(mid_target_property->name, name);
	
	if(status == 0)
            return mid; /* Return index of the found target */
	else if(status > 0)
	    right = mid - 1;
	else if(status < 0)
	    left = mid + 1;
    }
    
    return -1; /* Target not found */
}

gchar *get_target_property(const GArray *target, const gchar *name)
{
    int index = target_property_index(target, name);
    
    if(index == -1)
        return NULL;
    else
    {
        TargetProperty *targetProperty = g_array_index(target, TargetProperty*, index);
        return targetProperty->value;
    }
}

gchar *get_target_key(const GArray *target)
{
    gchar *target_property_name = get_target_property(target, "targetProperty");
    
    if(target_property_name == NULL)
        return NULL;
    else
        return get_target_property(target, target_property_name);
}

gchar **generate_activation_arguments(const GArray *target)
{
    unsigned int i;
    gchar **arguments = (gchar**)g_malloc((target->len + 1) * sizeof(gchar*));
    
    for(i = 0; i < target->len; i++)
    {
	TargetProperty *target_property = g_array_index(target, TargetProperty*, i);
	arguments[i] = g_strconcat(target_property->name, "=", target_property->value, NULL);
    }
    
    arguments[i] = NULL;
    
    return arguments;
}
