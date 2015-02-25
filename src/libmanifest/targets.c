/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2015  Sander van der Burg
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

static gint compare_target(const Target **l, const Target **r)
{
    const Target *left = *l;
    const Target *right = *r;
    
    gchar *left_target_property = find_target_key(left);
    gchar *right_target_property = find_target_key(right);
    
    return g_strcmp0(left_target_property, right_target_property);
}

static int compare_target_keys(const char *key, const Target **r)
{
    const Target *right = *r;
    gchar *right_target_property = find_target_key(right);
    
    return g_strcmp0(key, right_target_property);
}

static Target *create_target(void)
{
    Target *target = (Target*)g_malloc(sizeof(Target));
    target->properties = g_ptr_array_new();
    target->numOfCores = 1;
    target->availableCores = target->numOfCores;
    return target;
}

GPtrArray *generate_target_array(const gchar *manifest_file)
{
    /* Declarations */
    xmlDocPtr doc;
    xmlNodePtr node_root;
    xmlXPathObjectPtr result;
    GPtrArray *targets_array = NULL;
    
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
	targets_array = g_ptr_array_new();
	
	/* Iterate over all the target elements */
	for(i = 0; i < nodeset->nodeNr; i++)
	{
	    xmlNodePtr targets_children = nodeset->nodeTab[i]->children;
	    Target *target = create_target();
	    
	    while(targets_children != NULL)
	    {
	        TargetProperty *targetProperty = (TargetProperty*)g_malloc(sizeof(TargetProperty));
	        targetProperty->name = g_strdup((gchar*)targets_children->name);
	        
	        if(targets_children->children == NULL)
	            targetProperty->value = NULL;
	        else
	            targetProperty->value = g_strdup((gchar*)targets_children->children->content);
	        
	        g_ptr_array_add(target->properties, targetProperty);
	        
	        /* If the numOfCores properties has been encountered, set the value accordingly */
	        if(g_strcmp0(targetProperty->name, "numOfCores") == 0)
	        {
	            target->numOfCores = atoi(targetProperty->value);
	            target->availableCores = target->numOfCores;
	        }
	        
	        targets_children = targets_children->next;
	    }
	    
	    /* Sort the target properties */
	    g_ptr_array_sort(target->properties, (GCompareFunc)compare_target_property);
	    
	    /* Add target item to the targets array */
	    g_ptr_array_add(targets_array, target);
	}
	
	/* Sort the targets array */
	g_ptr_array_sort(targets_array, (GCompareFunc)compare_target);
	
	/* Cleanup */
	xmlXPathFreeObject(result);
    }
    
    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();
    
    /* Return the distribution array */
    return targets_array;
}

static void delete_target(Target *target)
{
    unsigned int i;
    
    for(i = 0; i < target->properties->len; i++)
    {
        TargetProperty *targetProperty = g_ptr_array_index(target->properties, i);
        
        g_free(targetProperty->name);
        g_free(targetProperty->value);
        g_free(targetProperty);
    }
    
    g_ptr_array_free(target->properties, TRUE);
    g_free(target);
}

void delete_target_array(GPtrArray *target_array)
{
    if(target_array != NULL)
    {
        unsigned int i;
        
        for(i = 0; i < target_array->len; i++)
        {
            Target *target = g_ptr_array_index(target_array, i);
            delete_target(target);
        }
    
        g_ptr_array_free(target_array, TRUE);
    }
}

void print_target_array(const GPtrArray *target_array)
{
    unsigned int i;
    
    for(i = 0; i < target_array->len; i++)
    {
        Target *target = g_ptr_array_index(target_array, i);
        unsigned int j;
        
        g_print("Target:\n");
        
        for(j = 0; j < target->properties->len; j++)
        {
            TargetProperty* targetProperty = g_ptr_array_index(target->properties, j);
            g_print("  %s = %s;\n", targetProperty->name, targetProperty->value);
        }
        
        g_print("\n");
    }
}

Target *find_target(const GPtrArray *target_array, const gchar *key)
{
    Target **ret = bsearch(key, target_array->pdata, target_array->len, sizeof(gpointer), (int (*)(const void *, const void *)) compare_target_keys);
    
    if(ret == NULL)
        return NULL;
    else
        return *ret;
}

gchar *find_target_property(const Target *target, const gchar *name)
{
    TargetProperty key;
    const TargetProperty *key_ptr = &key;
    TargetProperty **ret;
    
    key.name = (gchar*)name;
    
    ret = bsearch(&key_ptr, target->properties->pdata, target->properties->len, sizeof(gpointer), (int (*)(const void *, const void *)) compare_target_property);
    
    if(ret == NULL)
        return NULL;
    else
        return (*ret)->value;
}

gchar *find_target_key(const Target *target)
{
    gchar *target_property_name = find_target_property(target, "targetProperty");
    
    if(target_property_name == NULL)
        return NULL;
    else
        return find_target_property(target, target_property_name);
}

gchar *find_target_client_interface(const Target *target)
{
    return find_target_property(target, "clientInterface");
}

gchar **generate_activation_arguments(const Target *target)
{
    unsigned int i;
    gchar **arguments = (gchar**)g_malloc((target->properties->len + 1) * sizeof(gchar*));
    
    for(i = 0; i < target->properties->len; i++)
    {
	TargetProperty *target_property = g_ptr_array_index(target->properties, i);
	arguments[i] = g_strconcat(target_property->name, "=", target_property->value, NULL);
    }
    
    arguments[i] = NULL;
    
    return arguments;
}

int request_available_target_core(Target *target)
{
    if(target->availableCores > 0)
    {
        target->availableCores--;
        return TRUE;
    }
    else
        return FALSE;
}

void signal_available_target_core(Target *target)
{
    target->availableCores++;
}
