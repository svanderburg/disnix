/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2010  Sander van der Burg
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

static gint compare_activation_mapping(const ActivationMapping **l, const ActivationMapping **r)
{
    const ActivationMapping *left = *l;
    const ActivationMapping *right = *r;
    
    /* Compare the service names */
    gint status = g_strcmp0(left->service, right->service);

    /* If services are equal then compare the targets */    
    if(status == 0)
    {
	unsigned int i;
		
	for(i = 0; i < min(left->target->len, right->target->len); i++)
	{
	    TargetProperty *left_property = g_array_index(left->target, TargetProperty*, i);
	    TargetProperty *right_property = g_array_index(right->target, TargetProperty*, i);
	    
	    /* Compare the target property names */
	    status = g_strcmp0(left_property->name, right_property->name);
	    
	    /* If names are equal then compare the values */
	    if(status == 0)
	    {
		status = g_strcmp0(left_property->value, right_property->value);
		
		if(status != 0)
		    return status;
	    }
	    else
		return status;
	}
	
	/* The shortest property list takes precendence in case if the other target properties match */
	if(left->target->len < right->target->len)
	    return -1;
	else if(left->target->len > right->target->len)
	    return 1;
	else
	    return 0; /* Seems that both property lists are identical */
    }
    else
	return status;
}

gint activation_mapping_index(GArray *activation_array, ActivationMapping *keys)
{
    gint left = 0;
    gint right = activation_array->len - 1;
    
    while(left <= right)
    {
	gint mid = (left + right) / 2;
	ActivationMapping *mid_mapping = g_array_index(activation_array, ActivationMapping*, mid);
        gint status = compare_activation_mapping((const ActivationMapping**) &mid_mapping, (const ActivationMapping**) &keys);
	
	if(status == 0)
            return mid; /* Return index of the found activation mapping */
	else if(status > 0)
	    right = mid - 1;
	else if(status < 0)
	    left = mid + 1;
    }
    
    return -1; /* Activation mapping not found */
}

void print_activation_array(GArray *activation_array)
{
    unsigned int i;
    
    for(i = 0; i < activation_array->len; i++)
    {
	unsigned int j;
	ActivationMapping *mapping = g_array_index(activation_array, ActivationMapping*, i);
	
	g_print("service: %s\n", mapping->service);
	g_print("target:\n");
	
	for(j = 0; j < mapping->target->len; j++)
	{
	    TargetProperty *property = g_array_index(mapping->target, TargetProperty*, j);	    
	    g_print("  name: %s, value: %s\n", property->name, property->value);
	}
	
	g_print("targetProperty: %s\n", mapping->targetProperty);
	g_print("type: %s\n", mapping->type);
	g_print("dependsOn:\n");
	
	for(j = 0; j < mapping->depends_on->len; j++)
	{
	    unsigned int k;
	    Dependency *dependency = g_array_index(mapping->depends_on, Dependency*, j);
	    
	    g_print("  service: %s\n", dependency->service);
	    g_print("  target:\n");
	    
	    for(k = 0; k < dependency->target->len; k++)
	    {
		TargetProperty *property = g_array_index(dependency->target, TargetProperty*, k);
		g_print("    name: %s, value: %s\n", property->name, property->value);
	    }	    
	}
	
	g_print("\n");
    }
}

GArray *create_activation_array(char *manifest_file)
{
    xmlDocPtr doc;
    xmlNodePtr node_root;
    xmlXPathObjectPtr result;
    GArray *activation_array;
    unsigned int i;
    
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
    
    /* Initialize activation list */
    activation_array = g_array_new(FALSE, FALSE, sizeof(ActivationMapping*));
    
    /* Iterate over all the distribution elements and add them to the list */
    
    if(result)
    {
	xmlNodeSetPtr nodeset = result->nodesetval;
		
	/* Iterate over all the mapping elements */
	for(i = 0; i < nodeset->nodeNr; i++)
        {
	    xmlNodePtr mapping_children = nodeset->nodeTab[i]->children;
	    gchar *service;
	    GArray *target;
	    gchar *targetProperty;
	    gchar *type;
	    GArray *depends_on = NULL;
	    ActivationMapping *mapping = (ActivationMapping*)g_malloc(sizeof(ActivationMapping));
	    
	    /* Iterate over all the mapping item children (service,target,targetProperty,type,dependsOn elements) */
	    
	    while(mapping_children != NULL)
	    {
		if(xmlStrcmp(mapping_children->name, "service") == 0)
		    service = g_strdup(mapping_children->children->content);
		else if(xmlStrcmp(mapping_children->name, "targetProperty") == 0)
		    targetProperty = g_strdup(mapping_children->children->content);
		else if(xmlStrcmp(mapping_children->name, "type") == 0)
		    type = g_strdup(mapping_children->children->content);
		else if(xmlStrcmp(mapping_children->name, "target") == 0)
		{
		    xmlNodePtr target_children = mapping_children->children;
		    
		    target = g_array_new(FALSE, FALSE, sizeof(TargetProperty*));
		    
		    /* Iterate over the target children (target properties) */
		    
		    while(target_children != NULL)
		    {
			TargetProperty *property = (TargetProperty*)g_malloc(sizeof(TargetProperty));
			
			if(xmlStrcmp(target_children->name, (xmlChar*) "text") != 0) /* It seems that a text node is added when the manifest is pretty printed */
			{
			    property->name = g_strdup(target_children->name);
			    property->value = g_strdup(target_children->children->content);
			
			    g_array_append_val(target, property);
			}
			
			target_children = target_children->next;
		    }
		}
		else if(xmlStrcmp(mapping_children->name, "dependsOn") == 0)
		{
		    xmlNodePtr depends_on_children = mapping_children->children;
		    depends_on = g_array_new(FALSE, FALSE, sizeof(Dependency*));	    
		    
		    /* Iterate over all services in dependsOn (dependency element) */
		    while(depends_on_children != NULL)
		    {
			xmlNodePtr dependency_children = depends_on_children->children;
			gchar *service;
			GArray *target;
			Dependency *dependency = (Dependency*)g_malloc(sizeof(Dependency));
			
			if(xmlStrcmp(depends_on_children->name, "dependency") == 0) /* Only iterate over dependency nodes */
			{
			    /* Iterate over all dependency properties */
			    while(dependency_children != NULL)
			    {			    
				if(xmlStrcmp(dependency_children->name, (xmlChar*) "service") == 0)
				    service = g_strdup(dependency_children->children->content);
				else if(xmlStrcmp(dependency_children->name, (xmlChar*) "target") == 0)
				{
				    xmlNodePtr target_children = dependency_children->children;
		    
				    target = g_array_new(FALSE, FALSE, sizeof(TargetProperty*));
		    
				    /* Iterate over the target children (target properties) */
		    
				    while(target_children != NULL)
				    {
					TargetProperty *property = (TargetProperty*)g_malloc(sizeof(TargetProperty));
			
					if(xmlStrcmp(target_children->name, (xmlChar*) "text") != 0) /* It seems that a text node is added when the manifest is pretty printed */
					{
					    property->name = g_strdup(target_children->name);
					    property->value = g_strdup(target_children->children->content);
			
					    g_array_append_val(target, property);
					}
				    
					target_children = target_children->next;
				    }
				}

				dependency_children = dependency_children->next;
			    }			
			
			    dependency->service = service;
			    dependency->target = target;
			    g_array_append_val(depends_on, dependency);
			}
			
			depends_on_children = depends_on_children->next;
		    }
		}
						
		mapping_children = mapping_children->next;
	    }
	    
	    mapping->service = service;
	    mapping->target = target;
	    mapping->targetProperty = targetProperty;
	    mapping->type = type;
	    mapping->depends_on = depends_on;
		
	    /* Add the mapping to the list */
	    g_array_append_val(activation_array, mapping);
	}
    }

    /* Cleanup */
    xmlXPathFreeObject(result);
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Sort the activation list */
    g_array_sort(activation_array, (GCompareFunc)compare_activation_mapping);
    
    /* Return the activation list */
    return activation_array;
}

static void delete_target_array(GArray *target)
{
    unsigned int i;
    
    for(i = 0; i < target->len; i++)
    {
        TargetProperty *target_property = g_array_index(target, TargetProperty*, i);
        g_free(target_property->name);
        g_free(target_property->value);
        g_free(target_property);
    }
	
    g_array_free(target, TRUE);
}

void delete_activation_array(GArray *activation_array)
{
    unsigned int i;
    
    for(i = 0; i < activation_array->len; i++)
    {
	ActivationMapping *mapping = g_array_index(activation_array, ActivationMapping*, i);
	unsigned int j;
	
	g_free(mapping->service);
	delete_target_array(mapping->target);	
	g_free(mapping->targetProperty);
	g_free(mapping->type);
	
	for(j = 0; j < mapping->depends_on->len; j++)
	{
	    Dependency *dependency = g_array_index(mapping->depends_on, Dependency*, j);
	    g_free(dependency->service);
	    delete_target_array(dependency->target);
	    g_free(dependency);
	}
	
	g_array_free(mapping->depends_on, TRUE);
	g_free(mapping);
    }
    
    g_array_free(activation_array, TRUE);
}

GArray *intersect_activation_array(GArray *left, GArray *right)
{
    unsigned int i;
    GArray *return_list = g_array_new(FALSE, FALSE, sizeof(ActivationMapping*));
    
    if(left->len < right->len)
    {
	for(i = 0; i < left->len; i++)
	{
	    ActivationMapping *left_mapping = g_array_index(left, ActivationMapping*, i);
	    
	    if(activation_mapping_index(right, left_mapping) != -1)
		g_array_append_val(return_list, left_mapping);	    
	}
    }
    else
    {
	for(i = 0; i < right->len; i++)
	{
	    ActivationMapping *right_mapping = g_array_index(right, ActivationMapping*, i);

	    if(activation_mapping_index(left, right_mapping) != -1)
		g_array_append_val(return_list, right_mapping);	    
	}    
    }
    
    return return_list;
}

GArray *union_activation_array(GArray *left, GArray *right, GArray *intersect)
{    
    unsigned int i;
    GArray *return_list = g_array_new(FALSE, FALSE, sizeof(ActivationMapping*));
    
    /* Create a clone of the left list and mark mappings as activated */
    
    for(i = 0; i < left->len; i++)
    {
	ActivationMapping *mapping = g_array_index(left, ActivationMapping*, i);
	mapping->activated = TRUE;
	g_array_append_val(return_list, mapping);
    }
    
    /* Append all mappings from the right list which are not in the intersection and mark them as deactivated */
    
    for(i = 0; i < right->len; i++)
    {
	ActivationMapping *mapping = g_array_index(right, ActivationMapping*, i);
	mapping->activated = FALSE;
	
	if(activation_mapping_index(intersect, mapping) == -1)
	    g_array_append_val(return_list, mapping);
    }
    
    /* Sort the activation list */
    g_array_sort(return_list, (GCompareFunc)compare_activation_mapping);
    
    /* Return the activation list */
    return return_list;
}

GArray *substract_activation_array(GArray *left, GArray *right)
{
    unsigned int i;
    
    GArray *return_list = g_array_new(FALSE, FALSE, sizeof(ActivationMapping*));
    
    /* Create a clone of the left list */
    
    for(i = 0; i < left->len; i++)
    {
	ActivationMapping *mapping = g_array_index(left, ActivationMapping*, i);
	g_array_append_val(return_list, mapping);
    }
    
    /* Remove all mappings from the right list */
    
    for(i = 0; i < right->len; i++)
    {
	ActivationMapping *mapping = g_array_index(right, ActivationMapping*, i);
	gint index = activation_mapping_index(return_list, mapping);
	
	if(index != -1)
	    g_array_remove_index(return_list, index);
    }
    
    /* Return the activation list */
    return return_list;

}

gchar **generate_activation_arguments(GArray *target)
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

gchar *get_target_property(ActivationMapping *mapping)
{
    unsigned int i;
    
    for(i = 0; i < mapping->target->len; i++)
    {
	TargetProperty *target_property = g_array_index(mapping->target, TargetProperty*, i);
	
	if(g_strcmp0(target_property->name, mapping->targetProperty) == 0)
	    return target_property->value;
    }
    
    return NULL;
}

GArray *find_interdependent_mappings(GArray *list, ActivationMapping *mapping)
{
    unsigned int i;
    GArray *return_list = g_array_new(FALSE, FALSE, sizeof(ActivationMapping*));
    
    for(i = 0; i < list->len; i++)
    {
	unsigned int j;
	ActivationMapping *current_mapping = g_array_index(list, ActivationMapping*, i);
	
	for(j = 0; j < current_mapping->depends_on->len; j++)
	{
	    Dependency *dependency = g_array_index(current_mapping->depends_on, Dependency*, j);
	    ActivationMapping compare_mapping, *compare_mapping_ptr;

	    compare_mapping.service = dependency->service;
	    compare_mapping.target = dependency->target;
	    compare_mapping_ptr = &compare_mapping;
	
	    if(compare_activation_mapping((const ActivationMapping**) &mapping, (const ActivationMapping**) &compare_mapping_ptr) == 0)
		g_array_append_val(return_list, current_mapping);	    
	}
    }
        
    return return_list;
}
