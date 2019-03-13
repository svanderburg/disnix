/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2018  Sander van der Burg
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

static gchar *find_target_property(const Target *target, const gchar *name)
{
    if(target->properties_table == NULL || name == NULL)
        return NULL;
    else
        return g_hash_table_lookup(target->properties_table, name);
}

static gint compare_target(const Target **l, const Target **r)
{
    const Target *left = *l;
    const Target *right = *r;

    gchar *left_target_property = find_target_key(left, NULL);
    gchar *right_target_property = find_target_key(right, NULL);

    return g_strcmp0(left_target_property, right_target_property);
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

static gpointer parse_container(xmlNodePtr element)
{
    return parse_dictionary(element, parse_value);
}

static gpointer parse_target(xmlNodePtr element)
{
    Target *target = (Target*)g_malloc0(sizeof(Target));
    xmlNodePtr element_children = element->children;

    while(element_children != NULL)
    {
        if(xmlStrcmp(element_children->name, (xmlChar*) "name") == 0)
            target->name = parse_value(element_children);
        else if(xmlStrcmp(element_children->name, (xmlChar*) "system") == 0)
            target->system = parse_value(element_children);
        else if(xmlStrcmp(element_children->name, (xmlChar*) "clientInterface") == 0)
            target->client_interface = parse_value(element_children);
        else if(xmlStrcmp(element_children->name, (xmlChar*) "targetProperty") == 0)
            target->target_property = parse_value(element_children);
        else if(xmlStrcmp(element_children->name, (xmlChar*) "numOfCores") == 0)
        {
            gchar *num_of_cores_str = parse_value(element_children);

            if(num_of_cores_str != NULL)
            {
                target->num_of_cores = atoi((char*)num_of_cores_str);
                target->available_cores = target->num_of_cores;
                g_free(num_of_cores_str);
            }
        }
        else if(xmlStrcmp(element_children->name, (xmlChar*) "properties") == 0)
            target->properties_table = parse_dictionary(element_children, parse_value);
        else if(xmlStrcmp(element_children->name, (xmlChar*) "containers") == 0)
            target->containers_table = parse_dictionary(element_children, parse_container);

        element_children = element_children->next;
    }

    return target;
}

static GPtrArray *parse_targets(xmlNodePtr element)
{
    GPtrArray *targets_array = parse_list(element, "target", parse_target);
    g_ptr_array_sort(targets_array, (GCompareFunc)compare_target);
    return targets_array;
}

GPtrArray *create_target_array_from_doc(xmlDocPtr doc)
{
    xmlNodePtr node_root;
    GPtrArray *targets_array;

    if(doc == NULL)
    {
        g_printerr("Error with parsing infrastructure XML file!\n");
        return NULL;
    }

    /* Retrieve root element */
    node_root = xmlDocGetRootElement(doc);

    if(node_root == NULL)
    {
        g_printerr("The infrastructure XML file is empty!\n");
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }

    /* Parse targets */
    targets_array = parse_targets(node_root);

    /* Return targets */
    return targets_array;
}

GPtrArray *create_target_array_from_nix(char *infrastructure_expr)
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

GPtrArray *create_target_array_from_xml(const char *infrastructure_xml)
{
    /* Declarations */
    xmlDocPtr doc;
    GPtrArray *targets_array;

    /* Open the XML file */
    if((doc = xmlParseFile(infrastructure_xml)) == NULL)
    {
        g_printerr("Error with parsing the manifest XML file!\n");
        xmlCleanupParser();
        return NULL;
    }

    /* Create a target array from the XML document */
    targets_array = create_target_array_from_doc(doc);

    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Return the target array */
    return targets_array;
}

GPtrArray *create_target_array(gchar *infrastructure_expr, const int xml)
{
    if(xml)
        return create_target_array_from_xml(infrastructure_expr);
    else
        return create_target_array_from_nix(infrastructure_expr);
}

static void delete_properties_table(GHashTable *properties_table)
{
    if(properties_table != NULL)
    {
        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init(&iter, properties_table);
        while (g_hash_table_iter_next(&iter, &key, &value))
        {
            g_free(key);
            g_free(value);
        }

        g_hash_table_destroy(properties_table);
    }
}

static void delete_containers_table(GHashTable *containers_table)
{
    if(containers_table != NULL)
    {
        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init(&iter, containers_table);
        while (g_hash_table_iter_next(&iter, &key, &value))
        {
            GHashTable *container_table = (GHashTable*)value;
            g_free(key);
            delete_properties_table(container_table);
        }

        g_hash_table_destroy(containers_table);
    }
}

static void delete_target(Target *target)
{
    delete_properties_table(target->properties_table);
    delete_containers_table(target->containers_table);

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

int check_target_array(const GPtrArray *target_array)
{
    if(target_array == NULL)
        return TRUE;
    else
    {
        unsigned int i;

        for(i = 0; i < target_array->len; i++)
        {
            Target *target = g_ptr_array_index(target_array, i);

            if(target->properties_table == NULL)
            {
                /* Check if all mandatory properties have been provided */
                g_printerr("A mandatory property seems to be missing. Have you provided a correct\n");
                g_printerr("infrastrucure file?\n");
                return FALSE;
            }
        }
    }

    return TRUE;
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
    return has_next_iteration_process(&target_iterator_data->model_iterator_data);
}

static pid_t next_target_process(void *data)
{
    /* Declarations */
    pid_t pid;
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)data;
    
    /* Retrieve distributionitem, target pair */
    Target *target = g_ptr_array_index(target_iterator_data->target_array, target_iterator_data->model_iterator_data.index);
    gchar *client_interface = target->client_interface;
    gchar *target_key = find_target_key(target, target_iterator_data->target_property);

    /* If no client interface is provided by the infrastructure model, use global one */
    if(client_interface == NULL)
        client_interface = target_iterator_data->interface;
    
    /* Invoke the next distribution item operation process */
    pid = target_iterator_data->map_target_function.pid(target_iterator_data->data, target, client_interface, target_key);
    
    /* Increase the iterator index and update the pid table */
    next_iteration_process(&target_iterator_data->model_iterator_data, pid, target);
    
    /* Return the pid of the invoked process */
    return pid;
}

static void complete_target_process(void *data, pid_t pid, ProcReact_Status status, int result)
{
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)data;
    
    /* Retrieve the completed item */
    Target *target = complete_iteration_process(&target_iterator_data->model_iterator_data, pid, status, result);
    gchar *target_key = find_target_key(target, target_iterator_data->target_property);
    
    /* Invoke callback that handles completion of the target */
    target_iterator_data->complete_target_mapping_function.pid(target_iterator_data->data, target, target_key, status, result);
}

static ProcReact_Future next_target_future(void *data)
{
    /* Declarations */
    ProcReact_Future future;
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)data;
    
    /* Retrieve distributionitem, target pair */
    Target *target = g_ptr_array_index(target_iterator_data->target_array, target_iterator_data->model_iterator_data.index);
    gchar *client_interface = target->client_interface;
    gchar *target_key = find_target_key(target, target_iterator_data->target_property);

    /* If no client interface is provided by the infrastructure model, use global one */
    if(client_interface == NULL)
        client_interface = target_iterator_data->interface;
    
    /* Invoke the next distribution item operation process */
    future = target_iterator_data->map_target_function.future(target_iterator_data->data, target, client_interface, target_key);
    
    /* Increase the iterator index and update the pid table */
    next_iteration_future(&target_iterator_data->model_iterator_data, &future, target);
    
    /* Return the future of the invoked process */
    return future;
}

static void complete_target_future(void *data, ProcReact_Future *future, ProcReact_Status status)
{
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)data;
    
    /* Retrieve corresponding target and properties of the pid */
    Target *target = complete_iteration_future(&target_iterator_data->model_iterator_data, future, status);
    gchar *target_key = find_target_key(target, target_iterator_data->target_property);
    
    /* Invoke callback that handles completion of the target */
    target_iterator_data->complete_target_mapping_function.future(target_iterator_data->data, target, target_key, future, status);
}

static TargetIteratorData *create_common_iterator(GPtrArray *target_array, const gchar *target_property, gchar *interface, void *data)
{
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)g_malloc(sizeof(TargetIteratorData));
    
    init_model_iterator_data(&target_iterator_data->model_iterator_data, target_array->len);
    target_iterator_data->target_array = target_array;
    target_iterator_data->target_property = target_property;
    target_iterator_data->interface = interface;
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
    destroy_model_iterator_data(&target_iterator_data->model_iterator_data);
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
    return target_iterator_data->model_iterator_data.success;
}
