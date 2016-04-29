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

static gint compare_container(const Container **l, const Container **r)
{
    const Container *left = *l;
    const Container *right = *r;
    
    return g_strcmp0(left->name, right->name);
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
	    Target *target = (Target*)g_malloc(sizeof(Target));
	
	    gchar *system = NULL;
	    gchar *clientInterface = NULL;
	    gchar *targetProperty = NULL;
	    int numOfCores = 0;
	    int availableCores = 0;
	    GPtrArray *properties = NULL;
	    GPtrArray *containers = NULL;
	
	    while(targets_children != NULL)
	    {
	        if(xmlStrcmp(targets_children->name, (xmlChar*) "system") == 0)
	            system = g_strdup((gchar*)targets_children->children->content);
	        else if(xmlStrcmp(targets_children->name, (xmlChar*) "clientInterface") == 0)
	            clientInterface = g_strdup((gchar*)targets_children->children->content);
	        else if(xmlStrcmp(targets_children->name, (xmlChar*) "targetProperty") == 0)
	            targetProperty = g_strdup((gchar*)targets_children->children->content);
	        else if(xmlStrcmp(targets_children->name, (xmlChar*) "numOfCores") == 0)
	        {
	            numOfCores = atoi((char*)targets_children->children->content);
	            availableCores = numOfCores;
	        }
	        else if(xmlStrcmp(targets_children->name, (xmlChar*) "properties") == 0)
	        {
	            xmlNodePtr properties_children = targets_children->children;
	            properties = g_ptr_array_new();
	            
	            /* Iterate over all properties */
	            while(properties_children != NULL)
	            {
	                TargetProperty *targetProperty = (TargetProperty*)g_malloc(sizeof(TargetProperty));
	                targetProperty->name = g_strdup((gchar*)properties_children->name);
	                
	                if(properties_children->children == NULL)
	                    targetProperty->value = NULL;
	                else
	                    targetProperty->value = g_strdup((gchar*)properties_children->children->content);
	                
	                g_ptr_array_add(properties, targetProperty);
	                
	                properties_children = properties_children->next;
	            }
	            
	            /* Sort the target properties */
	            g_ptr_array_sort(properties, (GCompareFunc)compare_target_property);
	        }
	        else if(xmlStrcmp(targets_children->name, (xmlChar*) "containers") == 0)
	        {
	            xmlNodePtr container_children = targets_children->children;
	            containers = g_ptr_array_new();
	            
	            /* Iterate over all containers */
	            while(container_children != NULL)
	            {
	                Container *container = (Container*)g_malloc(sizeof(Container));
	                container->name = g_strdup((gchar*)container_children->name);
	                
	                if(container_children->children == NULL)
	                    container->properties = NULL;
	                else
	                {
	                    xmlNodePtr properties_children = container_children->children;
	                    GPtrArray *properties = g_ptr_array_new();
	                    
	                    /* Iterate over all properties */
	                    while(properties_children != NULL)
	                    {
	                        TargetProperty *targetProperty = (TargetProperty*)g_malloc(sizeof(TargetProperty));
	                        targetProperty->name = g_strdup((gchar*)properties_children->name);
	                        
	                        if(properties_children->children == NULL)
	                            targetProperty->value = NULL;
	                        else
	                            targetProperty->value = g_strdup((gchar*)properties_children->children->content);
	                
	                        g_ptr_array_add(properties, targetProperty);
	                
	                        properties_children = properties_children->next;
	                    }
	                    
	                    /* Sort the target properties */
	                    g_ptr_array_sort(properties, (GCompareFunc)compare_target_property);
	                    
	                    container->properties = properties;
	                }
	                
	                g_ptr_array_add(containers, container);
	                
	                container_children = container_children->next;
	            }
	            
	            /* Sort the containers */
	            g_ptr_array_sort(containers, (GCompareFunc)compare_container);
	        }
	        
	        targets_children = targets_children->next;
	    }
	    
	    target->system = system;
	    target->clientInterface = clientInterface;
	    target->targetProperty = targetProperty;
	    target->numOfCores = numOfCores;
	    target->availableCores = availableCores;
	    target->properties = properties;
	    target->containers = containers;
	    
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

static void delete_properties(GPtrArray *properties)
{
    if(properties != NULL)
    {
        unsigned int i;
    
        for(i = 0; i < properties->len; i++)
        {
            TargetProperty *targetProperty = g_ptr_array_index(properties, i);
            
            g_free(targetProperty->name);
            g_free(targetProperty->value);
            g_free(targetProperty);
        }
    
        g_ptr_array_free(properties, TRUE);
    }
}

static void delete_containers(GPtrArray *containers)
{
    if(containers != NULL)
    {
        unsigned int i;
        
        for(i = 0; i < containers->len; i++)
        {
            Container *container = g_ptr_array_index(containers, i);
            g_free(container->name);
            delete_properties(container->properties);
        }
        
        g_ptr_array_free(containers, TRUE);
    }
}

static void delete_target(Target *target)
{
    delete_properties(target->properties);
    delete_containers(target->containers);
    
    g_free(target->system);
    g_free(target->clientInterface);
    g_free(target->targetProperty);
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

static void print_properties(const GPtrArray *properties)
{
    unsigned int i;
    
    for(i = 0; i < properties->len; i++)
    {
        TargetProperty *targetProperty = g_ptr_array_index(properties, i);
        g_print("    %s = %s;\n", targetProperty->name, targetProperty->value);
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
        
        g_print("  properties:\n");
        print_properties(target->properties);
        
        g_print("  containers:\n");
        
        for(j = 0; j < target->containers->len; j++)
        {
            Container *container = g_ptr_array_index(target->containers, j);
            g_print("    %s:\n", container->name);
            print_properties(container->properties);
            g_print("\n");
        }
        
        g_print("  system = %s\n", target->system);
        g_print("  clientInterface = %s\n", target->clientInterface);
        g_print("  targetProperty = %s\n", target->targetProperty);
        g_print("  numOfCores = %d\n", target->numOfCores);
        
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
    return find_target_property(target, target->targetProperty);
}

static Container *find_container(const GPtrArray *containers, const gchar *name)
{
    if(containers == NULL)
        return NULL;
    else
    {
        Container key;
        const Container *key_ptr = &key;
        Container **ret;
        
        key.name = (gchar*)name;
        
        ret = bsearch(&key_ptr, containers->pdata, containers->len, sizeof(gpointer), (int (*)(const void *, const void *)) compare_container);
        
        if(ret == NULL)
            return NULL;
        else
            return *ret;
    }
}

gchar **generate_activation_arguments(const Target *target, const gchar *container_name)
{
    Container *container = find_container(target->containers, container_name);
    
    if(container == NULL || container->properties == NULL)
        return NULL;
    else
    {
        unsigned int i;
        gchar **arguments = (gchar**)g_malloc((container->properties->len + 1) * sizeof(gchar*));
        
        for(i = 0; i < container->properties->len; i++)
        {
            TargetProperty *container_property = g_ptr_array_index(container->properties, i);
            arguments[i] = g_strconcat(container_property->name, "=", container_property->value, NULL);
        }
        
        arguments[i] = NULL;
        
        return arguments;
    }
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
