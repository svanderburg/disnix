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

#include "activationmapping.h"
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <xmlutil.h>
#define min(a,b) ((a) < (b) ? (a) : (b))

static gint compare_activation_mapping_keys(const ActivationMappingKey **l, const ActivationMappingKey **r)
{
    const ActivationMappingKey *left = *l;
    const ActivationMappingKey *right = *r;
    
    /* Compare the service keys */
    gint status = g_strcmp0(left->key, right->key);
    
    if(status == 0)
        return g_strcmp0(left->target, right->target); /* If services are equal then compare the targets */
    else
        return status;
}

static gint compare_activation_mapping(const ActivationMapping **l, const ActivationMapping **r)
{
    return compare_activation_mapping_keys((const ActivationMappingKey **)l, (const ActivationMappingKey **)r);
}

gint activation_mapping_index(const GArray *activation_array, gchar *key, gchar *target)
{
    gint left = 0;
    gint right = activation_array->len - 1;
     
    while(left <= right)
    {
       gint mid = (left + right) / 2;
       const ActivationMapping *mid_mapping = g_array_index(activation_array, ActivationMapping*, mid);
       ActivationMapping keys;
       const ActivationMapping *keys_ptr = &keys;
       gint status;
       
       keys.key = key;
       keys.target = target;
       
        status = compare_activation_mapping(&mid_mapping, &keys_ptr);
       
       if(status == 0)
            return mid; /* Return index of the found activation mapping */
       else if(status > 0)
           right = mid - 1;
       else if(status < 0)
           left = mid + 1;
    }
    
    return -1; /* Activation mapping not found */
}

ActivationMapping *get_activation_mapping(const GArray *activation_array, gchar *key, gchar *target)
{
    /* Search for the location of the mapping in the union array */
    gint actual_mapping_index = activation_mapping_index(activation_array, key, target);
    
    if(actual_mapping_index == -1)
        return NULL;
    else
        return g_array_index(activation_array, ActivationMapping*, actual_mapping_index);
}

gint dependency_index(const GArray *depends_on, gchar *key, gchar *target)
{
    gint left = 0;
    gint right = depends_on->len - 1;
    
    while(left <= right)
    {
	gint mid = (left + right) / 2;
	const ActivationMappingKey *mid_dependency = g_array_index(depends_on, ActivationMappingKey*, mid);
	ActivationMappingKey keys;
	const ActivationMappingKey *key_ptr = &keys;
	gint status;
	
	keys.key = key;
	keys.target = target;
	
        status = compare_activation_mapping_keys(&mid_dependency, &key_ptr);
	
	if(status == 0)
            return mid; /* Return index of the found activation mapping */
	else if(status > 0)
	    right = mid - 1;
	else if(status < 0)
	    left = mid + 1;
    }
    
    return -1; /* Activation mapping not found */
}

ActivationMappingKey *get_dependency(const GArray *depends_on, gchar *key, gchar *target)
{
    /* Search for the location of the mapping in the union array */
    gint actual_dependency_index = dependency_index(depends_on, key, target);
    
    if(actual_dependency_index == -1)
        return NULL;
    else
        return g_array_index(depends_on, ActivationMappingKey*, actual_dependency_index);
}

void print_activation_array(const GArray *activation_array)
{
    unsigned int i;
    
    for(i = 0; i < activation_array->len; i++)
    {
	ActivationMapping *mapping = g_array_index(activation_array, ActivationMapping*, i);
	unsigned int j;
	
	g_print("key: %s\n", mapping->key);
	g_print("target: %s\n", mapping->target);
	g_print("service: %s\n", mapping->service);
	g_print("targetProperty: %s\n", mapping->targetProperty);
	g_print("type: %s\n", mapping->type);
	g_print("dependsOn:\n");
	
	for(j = 0; j < mapping->depends_on->len; j++)
	{
	    ActivationMappingKey *dependency = g_array_index(mapping->depends_on, ActivationMappingKey*, j);
	    
	    g_print("  key: %s\n", dependency->key);
	    g_print("  target: %s\n", mapping->target);
	}
	
	g_print("\n");
    }
}

GArray *create_activation_array(const gchar *manifest_file)
{
    xmlDocPtr doc;
    xmlNodePtr node_root;
    xmlXPathObjectPtr result;
    GArray *activation_array;
    
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
    result = executeXPathQuery(doc, "/manifest/activation/mapping");
    
    /* Initialize activation array */
    activation_array = g_array_new(FALSE, FALSE, sizeof(ActivationMapping*));
    
    /* Iterate over all the distribution elements and add them to the array */
    
    if(result)
    {
	xmlNodeSetPtr nodeset = result->nodesetval;
	unsigned int i;
	
	/* Iterate over all the mapping elements */
	for(i = 0; i < nodeset->nodeNr; i++)
        {
	    xmlNodePtr mapping_children = nodeset->nodeTab[i]->children;
	    gchar *key = NULL;
	    gchar *target = NULL;
	    gchar *service = NULL;
	    gchar *targetProperty = NULL;
	    gchar *name = NULL;
	    gchar *type = NULL;
	    GArray *depends_on = NULL;
	    ActivationMapping *mapping = (ActivationMapping*)g_malloc(sizeof(ActivationMapping));
	    
	    /* Iterate over all the mapping item children (service,target,targetProperty,type,dependsOn elements) */
	    
	    while(mapping_children != NULL)
	    {
		if(xmlStrcmp(mapping_children->name, "key") == 0)
		    key = g_strdup(mapping_children->children->content);
		else if(xmlStrcmp(mapping_children->name, "service") == 0)
		    service = g_strdup(mapping_children->children->content);
		else if(xmlStrcmp(mapping_children->name, "targetProperty") == 0)
		    targetProperty = g_strdup(mapping_children->children->content);
		else if(xmlStrcmp(mapping_children->name, "name") == 0)
		    name = g_strdup(mapping_children->children->content);
		else if(xmlStrcmp(mapping_children->name, "type") == 0)
		    type = g_strdup(mapping_children->children->content);
		else if(xmlStrcmp(mapping_children->name, "target") == 0)
		    target = g_strdup(mapping_children->children->content);
		else if(xmlStrcmp(mapping_children->name, "dependsOn") == 0)
		{
		    xmlNodePtr depends_on_children = mapping_children->children;
		    depends_on = g_array_new(FALSE, FALSE, sizeof(ActivationMappingKey*));
		    
		    /* Iterate over all services in dependsOn (dependency element) */
		    while(depends_on_children != NULL)
		    {
			xmlNodePtr dependency_children = depends_on_children->children;
			gchar *key = NULL;
			gchar *target = NULL;
			ActivationMappingKey *dependency = (ActivationMappingKey*)g_malloc(sizeof(ActivationMappingKey));
			
			if(xmlStrcmp(depends_on_children->name, "dependency") == 0) /* Only iterate over dependency nodes */
			{
			    /* Iterate over all dependency properties */
			    while(dependency_children != NULL)
			    {
				if(xmlStrcmp(dependency_children->name, (xmlChar*) "key") == 0)
				    key = g_strdup(dependency_children->children->content);
				else if(xmlStrcmp(dependency_children->name, (xmlChar*) "target") == 0)
				    target = g_strdup(dependency_children->children->content);
				    
				dependency_children = dependency_children->next;
			    }
			
			    dependency->key = key;
			    dependency->target = target;
			    g_array_append_val(depends_on, dependency);
			}
			
			depends_on_children = depends_on_children->next;
		    }
		    
		    /* Sort the dependency array */
		    g_array_sort(depends_on, (GCompareFunc)compare_activation_mapping_keys);
		}
		
		mapping_children = mapping_children->next;
	    }
	    
	    mapping->key = key;
	    mapping->target = target;
	    mapping->service = service;
	    mapping->targetProperty = targetProperty;
	    mapping->name = name;
	    mapping->type = type;
	    mapping->depends_on = depends_on;
		
	    /* Add the mapping to the array */
	    g_array_append_val(activation_array, mapping);
	}
    }

    /* Cleanup */
    xmlXPathFreeObject(result);
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Sort the activation array */
    g_array_sort(activation_array, (GCompareFunc)compare_activation_mapping);
    
    /* Return the activation array */
    return activation_array;
}

void delete_activation_array(GArray *activation_array)
{
    if(activation_array != NULL)
    {
        unsigned int i;
    
        for(i = 0; i < activation_array->len; i++)
        {
            ActivationMapping *mapping = g_array_index(activation_array, ActivationMapping*, i);
            unsigned int j;
            
            g_free(mapping->key);
            g_free(mapping->target);
            g_free(mapping->service);
            g_free(mapping->targetProperty);
            g_free(mapping->name);
            g_free(mapping->type);

            if(mapping->depends_on != NULL)
            {
                for(j = 0; j < mapping->depends_on->len; j++)
                {
                    ActivationMappingKey *dependency = g_array_index(mapping->depends_on, ActivationMappingKey*, j);
                    g_free(dependency->key);
                    g_free(dependency->target);
                    g_free(dependency);
                }
            }
            
            g_array_free(mapping->depends_on, TRUE);
            g_free(mapping);
        }
    
        g_array_free(activation_array, TRUE);
    }
}

GArray *intersect_activation_array(GArray *left, GArray *right)
{
    unsigned int i;
    GArray *return_array = g_array_new(FALSE, FALSE, sizeof(ActivationMapping*));
    
    if(left->len < right->len)
    {
	for(i = 0; i < left->len; i++)
	{
	    ActivationMapping *left_mapping = g_array_index(left, ActivationMapping*, i);
	    
	    if(activation_mapping_index(right, left_mapping->key, left_mapping->target) != -1)
		g_array_append_val(return_array, left_mapping);
	}
    }
    else
    {
	for(i = 0; i < right->len; i++)
	{
	    ActivationMapping *right_mapping = g_array_index(right, ActivationMapping*, i);

	    if(activation_mapping_index(left, right_mapping->key, right_mapping->target) != -1)
		g_array_append_val(return_array, right_mapping);
	}
    }
    
    return return_array;
}

GArray *union_activation_array(GArray *left, GArray *right, GArray *intersect)
{    
    unsigned int i;
    GArray *return_array = g_array_new(FALSE, FALSE, sizeof(ActivationMapping*));
    
    /* Create a clone of the left array and mark mappings as activated */
    
    for(i = 0; i < left->len; i++)
    {
	ActivationMapping *mapping = g_array_index(left, ActivationMapping*, i);
	mapping->activated = TRUE;
	g_array_append_val(return_array, mapping);
    }
    
    /* Append all mappings from the right array which are not in the intersection and mark them as deactivated */
    
    for(i = 0; i < right->len; i++)
    {
	ActivationMapping *mapping = g_array_index(right, ActivationMapping*, i);
	mapping->activated = FALSE;
	
	if(activation_mapping_index(intersect, mapping->key, mapping->target) == -1)
	    g_array_append_val(return_array, mapping);
    }
    
    /* Sort the activation array */
    g_array_sort(return_array, (GCompareFunc)compare_activation_mapping);
    
    /* Return the activation array */
    return return_array;
}

GArray *substract_activation_array(GArray *left, GArray *right)
{
    unsigned int i;
    GArray *return_array = g_array_new(FALSE, FALSE, sizeof(ActivationMapping*));
    
    /* Create a clone of the left array */
    
    for(i = 0; i < left->len; i++)
    {
	ActivationMapping *mapping = g_array_index(left, ActivationMapping*, i);
	g_array_append_val(return_array, mapping);
    }
    
    /* Remove all mappings from the right array */
    
    for(i = 0; i < right->len; i++)
    {
	ActivationMapping *mapping = g_array_index(right, ActivationMapping*, i);
	gint index = activation_mapping_index(return_array, mapping->key, mapping->target);
	
	if(index != -1)
	    g_array_remove_index(return_array, index);
    }
    
    /* Return the activation array */
    return return_array;
}

GArray *find_interdependent_mappings(GArray *activation_array, const ActivationMapping *mapping)
{
    GArray *return_array = g_array_new(FALSE, FALSE, sizeof(ActivationMapping*));
    unsigned int i;
    
    for(i = 0; i < activation_array->len; i++)
    {
	unsigned int j;
	ActivationMapping *current_mapping = g_array_index(activation_array, ActivationMapping*, i);
	ActivationMappingKey *found_dependency = get_dependency(current_mapping->depends_on, mapping->key, mapping->target);
	
	if(found_dependency != NULL)
	    g_array_append_val(return_array, current_mapping);
    }
    
    return return_array;
}
