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

#include "infrastructure.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <libxslt/xslt.h>
#include <libxslt/transform.h>
#include "package-management.h"

static gint compare_target_property(const TargetProperty **l, const TargetProperty **r)
{
    const TargetProperty *left = *l;
    const TargetProperty *right = *r;
    
    return g_strcmp0(left->name, right->name);
}

static gchar *find_target_property(const Target *target, const gchar *name)
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

static gint compare_target(const Target **l, const Target **r)
{
    const Target *left = *l;
    const Target *right = *r;
    
    gchar *left_target_property = find_target_key(left, NULL);
    gchar *right_target_property = find_target_key(right, NULL);
    
    return g_strcmp0(left_target_property, right_target_property);
}

static gint compare_container(const Container **l, const Container **r)
{
    const Container *left = *l;
    const Container *right = *r;
    
    return g_strcmp0(left->name, right->name);
}

static xmlDocPtr create_infrastructure_doc(gchar *infrastructureXML)
{
    /* Declarations */
    xmlDocPtr doc, transform_doc;
    xmlNodePtr root_node;
    xsltStylesheetPtr style;
    
    /* Parse XML file from XML string */
    doc = xmlParseMemory(infrastructureXML, strlen(infrastructureXML));
    
    if(doc == NULL)
    {
	g_printerr("Error with parsing infrastructure XML file!\n");
	return NULL;
    }
    
    /* Check if the document has a root */
    root_node = xmlDocGetRootElement(doc);
    
    if(root_node == NULL)
    {
        g_printerr("The infrastructure XML file is empty!\n");
	xmlFreeDoc(doc);
	return NULL;
    }

    /* Transform the document into a more concrete format */
    style = xsltParseStylesheetFile((const xmlChar *) DATADIR "/infrastructure.xsl");
    
    transform_doc = xsltApplyStylesheet(style, doc, NULL);
        
    /* Cleanup */
    xsltFreeStylesheet(style);
    xmlFreeDoc(doc);
    xsltCleanupGlobals();
    
    /* Return transformed XML document */
    return transform_doc;
}

GPtrArray *create_target_array_from_doc(xmlDocPtr doc)
{
    xmlXPathObjectPtr result;
    
    if(doc == NULL)
    {
        g_printerr("Error with parsing infrastructure XML file!\n");
        return NULL;
    }

    /* Query all the target properties from the XML document */
    result = executeXPathQuery(doc, "/infrastructure/target");

    /* Iterate over all targets and add them to the array */
    
    if(result)
    {
        unsigned int i;
        
        xmlNodeSetPtr nodeset = result->nodesetval;
        
        /* Create a targets array */
	GPtrArray *targets_array = g_ptr_array_new();
	
	/* Iterate over all the target elements */
	for(i = 0; i < nodeset->nodeNr; i++)
	{
	    xmlNodePtr targets_children = nodeset->nodeTab[i]->children;
	    xmlAttrPtr target_properties = nodeset->nodeTab[i]->properties;
	    Target *target = (Target*)g_malloc(sizeof(Target));
	
	    gchar *name = NULL;
	    gchar *system = NULL;
	    gchar *client_interface = NULL;
	    gchar *target_property = NULL;
	    int num_of_cores = 0;
	    int available_cores = 0;
	    GPtrArray *properties = NULL;
	    GPtrArray *containers = NULL;
	    
	    while(target_properties != NULL)
	    {
	        /* Parse the name attribute */
	        if(xmlStrcmp(target_properties->name, (xmlChar*) "name") == 0)
	        {
	           if(target_properties->children != NULL)
	               name = g_strdup((gchar*)target_properties->children->content);
	        }
	        
	        target_properties = target_properties->next;
	    }
	    
	    /* Parse the sub elements */
	    while(targets_children != NULL)
	    {
	        if(xmlStrcmp(targets_children->name, (xmlChar*) "system") == 0)
	            system = duplicate_node_text(targets_children);
	        else if(xmlStrcmp(targets_children->name, (xmlChar*) "clientInterface") == 0)
	            client_interface = duplicate_node_text(targets_children);
	        else if(xmlStrcmp(targets_children->name, (xmlChar*) "targetProperty") == 0)
	            target_property = duplicate_node_text(targets_children);
	        else if(xmlStrcmp(targets_children->name, (xmlChar*) "numOfCores") == 0)
	        {
	            gchar *num_of_cores_str = duplicate_node_text(targets_children);
	            
	            if(num_of_cores_str != NULL)
	            {
	                num_of_cores = atoi((char*)num_of_cores_str);
	                available_cores = num_of_cores;
	                g_free(num_of_cores_str);
	            }
	        }
	        else if(xmlStrcmp(targets_children->name, (xmlChar*) "properties") == 0)
	        {
	            xmlNodePtr properties_children = targets_children->children;
	            properties = g_ptr_array_new();
	            
	            /* Iterate over all properties */
	            while(properties_children != NULL)
	            {
	                TargetProperty *target_property = (TargetProperty*)g_malloc(sizeof(TargetProperty));
	                target_property->name = g_strdup((gchar*)properties_children->name);
	                target_property->value = duplicate_node_text(properties_children);
	                
	                g_ptr_array_add(properties, target_property);
	                
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
	                        TargetProperty *target_property = (TargetProperty*)g_malloc(sizeof(TargetProperty));
	                        target_property->name = g_strdup((gchar*)properties_children->name);
	                        target_property->value = duplicate_node_text(properties_children);
	                
	                        g_ptr_array_add(properties, target_property);
	                
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
	    
	    target->name = name;
	    target->system = system;
	    target->client_interface = client_interface;
	    target->target_property = target_property;
	    target->num_of_cores = num_of_cores;
	    target->available_cores = available_cores;
	    target->properties = properties;
	    target->containers = containers;
	    
	    /* Add target item to the targets array */
	    g_ptr_array_add(targets_array, target);
	}
	
	/* Sort the targets array */
	g_ptr_array_sort(targets_array, (GCompareFunc)compare_target);

	/* Cleanup */
	xmlXPathFreeObject(result);
	
	/* Return the generated targets array */
	return targets_array;
    }
    else
    {
        g_printerr("No targets found!\n");
        return NULL;
    }
}

GPtrArray *create_target_array(char *infrastructure_expr)
{
    /* Declarations */
    xmlDocPtr doc;
    GPtrArray *targets_array = NULL;
    
    /* Open the XML output of nix-instantiate */
    char *infrastructureXML = pkgmgmt_instantiate_sync(infrastructure_expr);
    
    if(infrastructureXML == NULL)
    {
        g_printerr("Error opening infrastructure XML file!\n");
        return NULL;
    }
    
    /* Parse the infrastructure XML file */
    doc = create_infrastructure_doc(infrastructureXML);
    
    /* Create a target array from the XML document */
    targets_array = create_target_array_from_doc(doc);

    /* Cleanup */
    free(infrastructureXML);
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Return the target array */
    return targets_array;
}

static void delete_properties(GPtrArray *properties)
{
    if(properties != NULL)
    {
        unsigned int i;
    
        for(i = 0; i < properties->len; i++)
        {
            TargetProperty *target_property = g_ptr_array_index(properties, i);
            
            g_free(target_property->name);
            g_free(target_property->value);
            g_free(target_property);
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
            g_free(container);
        }
        
        g_ptr_array_free(containers, TRUE);
    }
}

static void delete_target(Target *target)
{
    delete_properties(target->properties);
    delete_containers(target->containers);
    
    g_free(target->name);
    g_free(target->system);
    g_free(target->client_interface);
    g_free(target->target_property);
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

gchar *find_target_key(const Target *target, const gchar *global_target_property)
{
    if(target->target_property == NULL)
        return find_target_property(target, global_target_property);
    else
        return find_target_property(target, target->target_property);
}

static int has_next_target(void *data)
{
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)data;
    return target_iterator_data->index < target_iterator_data->length;
}

static pid_t next_target_process(void *data)
{
    /* Declarations */
    pid_t pid;
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)data;
    
    /* Retrieve distributionitem, target pair */
    Target *target = g_ptr_array_index(target_iterator_data->target_array, target_iterator_data->index);
    gchar *client_interface = target->client_interface;
    gchar *target_key = find_target_key(target, target_iterator_data->target_property);

    /* If no client interface is provided by the infrastructure model, use global one */
    if(client_interface == NULL)
        client_interface = target_iterator_data->interface;
    
    /* Invoke the next distribution item operation process */
    pid = target_iterator_data->map_target_function.pid(target_iterator_data->data, target, client_interface, target_key);
    
    /* Increase the iterator index */
    target_iterator_data->index++;
    
    if(pid > 0)
    {
        /* Add pid to the pid table so that we know what the corresponding target is */
        gint *pid_ptr = g_malloc(sizeof(gint));
        *pid_ptr = pid;
        g_hash_table_insert(target_iterator_data->pid_table, pid_ptr, target);
    }
    
    /* Return the pid of the invoked process */
    return pid;
}

static void complete_target_process(void *data, pid_t pid, ProcReact_Status status, int result)
{
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)data;
    
    /* Retrieve corresponding target and properties of the pid */
    gint *pid_ptr = &pid;
    Target *target = g_hash_table_lookup(target_iterator_data->pid_table, pid_ptr);
    gchar *target_key = find_target_key(target, target_iterator_data->target_property);
    
    /* If anything failed set the overall success status to FALSE */
    if(status != PROCREACT_STATUS_OK || !result)
        target_iterator_data->success = FALSE;
    
    /* Invoke callback that handles completion of the target */
    target_iterator_data->complete_target_mapping_function.pid(target_iterator_data->data, target, target_key, status, result);
}

static ProcReact_Future next_target_future(void *data)
{
    /* Declarations */
    ProcReact_Future future;
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)data;
    
    /* Retrieve distributionitem, target pair */
    Target *target = g_ptr_array_index(target_iterator_data->target_array, target_iterator_data->index);
    gchar *client_interface = target->client_interface;
    gchar *target_key = find_target_key(target, target_iterator_data->target_property);

    /* If no client interface is provided by the infrastructure model, use global one */
    if(client_interface == NULL)
        client_interface = target_iterator_data->interface;
    
    /* Invoke the next distribution item operation process */
    future = target_iterator_data->map_target_function.future(target_iterator_data->data, target, client_interface, target_key);
    
    /* Increase the iterator index */
    target_iterator_data->index++;
    
    if(future.pid > 0)
    {
        /* Add pid to the pid table so that we know what the corresponding target is */
        gint *pid_ptr = g_malloc(sizeof(gint));
        *pid_ptr = future.pid;
        g_hash_table_insert(target_iterator_data->pid_table, pid_ptr, target);
    }
    
    /* Return the future of the invoked process */
    return future;
}

static void complete_target_future(void *data, ProcReact_Future *future, ProcReact_Status status)
{
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)data;
    
    /* Retrieve corresponding target and properties of the pid */
    gint *pid_ptr = &future->pid;
    Target *target = g_hash_table_lookup(target_iterator_data->pid_table, pid_ptr);
    gchar *target_key = find_target_key(target, target_iterator_data->target_property);
    
    /* If anything failed set the overall success status to FALSE */
    if(status != PROCREACT_STATUS_OK || future->result == NULL)
        target_iterator_data->success = FALSE;
    
    /* Invoke callback that handles completion of the target */
    target_iterator_data->complete_target_mapping_function.future(target_iterator_data->data, target, target_key, future, status);
}

static TargetIteratorData *create_common_iterator(GPtrArray *target_array, const gchar *target_property, gchar *interface, void *data)
{
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)g_malloc(sizeof(TargetIteratorData));
    
    target_iterator_data->index = 0;
    target_iterator_data->length = target_array->len;
    target_iterator_data->success = TRUE;
    target_iterator_data->target_array = target_array;
    target_iterator_data->target_property = target_property;
    target_iterator_data->interface = interface;
    target_iterator_data->pid_table = g_hash_table_new_full(g_int_hash, g_int_equal, g_free, NULL);
    target_iterator_data->data = data;
    
    return target_iterator_data;
}

ProcReact_PidIterator create_target_pid_iterator(GPtrArray *target_array, const gchar *target_property, gchar *interface, map_target_pid_function map_target, complete_target_mapping_pid_function complete_target_mapping, void *data)
{
    TargetIteratorData *target_iterator_data = create_common_iterator(target_array, target_property, interface, data);
    
    target_iterator_data->map_target_function.pid = map_target;
    target_iterator_data->complete_target_mapping_function.pid = complete_target_mapping;
    
    return procreact_initialize_pid_iterator(has_next_target, next_target_process, procreact_retrieve_boolean, complete_target_process, target_iterator_data);
}

ProcReact_FutureIterator create_target_future_iterator(GPtrArray *target_array, const gchar *target_property, gchar *interface, map_target_future_function map_target, complete_target_mapping_future_function complete_target_mapping, void *data)
{
    TargetIteratorData *target_iterator_data = create_common_iterator(target_array, target_property, interface, data);
    
    target_iterator_data->map_target_function.future = map_target;
    target_iterator_data->complete_target_mapping_function.future = complete_target_mapping;
    
    return procreact_initialize_future_iterator(has_next_target, next_target_future, complete_target_future, target_iterator_data);
}

static void destroy_target_iterator_data(TargetIteratorData *target_iterator_data)
{
    g_hash_table_destroy(target_iterator_data->pid_table);
    g_free(target_iterator_data);
}

void destroy_target_pid_iterator(ProcReact_PidIterator *iterator)
{
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)iterator->data;
    destroy_target_iterator_data(target_iterator_data);
}

void destroy_target_future_iterator(ProcReact_FutureIterator *iterator)
{
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)iterator->data;
    destroy_target_iterator_data(target_iterator_data);
    procreact_destroy_future_iterator(iterator);
}

int target_iterator_has_succeeded(const TargetIteratorData *target_iterator_data)
{
    return target_iterator_data->success;
}
