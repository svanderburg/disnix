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

#include "targets.h"
#include <stdlib.h>
#include <xmlutil.h>

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
        if(xmlStrcmp(element_children->name, (xmlChar*) "system") == 0)
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

GPtrArray *parse_targets(xmlNodePtr element)
{
    GPtrArray *targets_array = parse_list(element, "target", parse_target);
    g_ptr_array_sort(targets_array, (GCompareFunc)compare_target);
    return targets_array;
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
    if(target != NULL)
    {
        delete_properties_table(target->properties_table);
        delete_containers_table(target->containers_table);
        
        g_free(target->system);
        g_free(target->client_interface);
        g_free(target->target_property);
        g_free(target);
    }
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
            if(target->system == NULL || target->client_interface == NULL || target->target_property == NULL)
            {
                /* Check if all mandatory properties have been provided */
                g_printerr("A mandatory property seems to be missing. Have you provided a correct\n");
                g_printerr("manifest file?\n");
                return FALSE;
            }
        }

        return TRUE;
    }
}

static void print_properties_table(GHashTable *properties_table)
{
    g_print("  properties:\n");

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, properties_table);
    while (g_hash_table_iter_next(&iter, &key, &value))
        g_print("    %s = %s;\n", (gchar*)key, (gchar*)value);
}

static void print_containers_table(GHashTable *containers_table)
{
    g_print("  containers:\n");

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, containers_table);
    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        GHashTable *container_table = (GHashTable*)value;
        g_print("    %s:\n", (gchar*)key);
        print_properties_table(container_table);
        g_print("\n");
    }
}

static void print_reserved_properties(const Target *target)
{
    g_print("  system = %s\n", target->system);
    g_print("  clientInterface = %s\n", target->client_interface);
    g_print("  targetProperty = %s\n", target->target_property);
    g_print("  numOfCores = %d\n", target->num_of_cores);
}

void print_target_array(const GPtrArray *target_array)
{
    unsigned int i;
    
    for(i = 0; i < target_array->len; i++)
    {
        Target *target = g_ptr_array_index(target_array, i);
        
        g_print("Target:\n");
        
        print_properties_table(target->properties_table);
        print_containers_table(target->containers_table);
        print_reserved_properties(target);
        
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
    if(target->properties_table == NULL)
        return NULL;
    else
        return g_hash_table_lookup(target->properties_table, name);
}

gchar *find_target_key(const Target *target)
{
    return find_target_property(target, target->target_property);
}

static GHashTable *find_container(GHashTable *containers_table, const gchar *name)
{
    if(containers_table == NULL)
        return NULL;
    else
        return g_hash_table_lookup(containers_table, name);
}

gchar **generate_activation_arguments(const Target *target, const gchar *container_name)
{
    GHashTable *container_table = find_container(target->containers_table, container_name);

    if(container_table == NULL)
    {
        gchar **ret = (gchar**)g_malloc(sizeof(gchar*));
        ret[0] = NULL;
        return ret;
    }
    else
    {
        unsigned int i = 0;
        gchar **arguments = (gchar**)g_malloc((g_hash_table_size(container_table) + 1) * sizeof(gchar*));
        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init(&iter, container_table);
        while (g_hash_table_iter_next(&iter, &key, &value))
        {
            arguments[i] = g_strconcat((gchar*)key, "=", (gchar*)value, NULL);
            i++;
        }

        arguments[i] = NULL;

        return arguments;
    }
}

int request_available_target_core(Target *target)
{
    if(target->available_cores > 0)
    {
        target->available_cores--;
        return TRUE;
    }
    else
        return FALSE;
}

void signal_available_target_core(Target *target)
{
    target->available_cores++;
}

static int has_next_target(void *data)
{
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)data;
    return has_next_iteration_process(&target_iterator_data->model_iterator_data);
}

static pid_t next_target_process(void *data)
{
    /* Declarations */
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)data;
    
    /* Retrieve target */
    Target *target = g_ptr_array_index(target_iterator_data->target_array, target_iterator_data->model_iterator_data.index);
    
    /* Invoke the next distribution item operation process */
    pid_t pid = target_iterator_data->map_target(target_iterator_data->data, target);
    
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
    
    /* Invoke callback that handles completion of the target */
    target_iterator_data->complete_target_mapping(target_iterator_data->data, target, status, result);
}

ProcReact_PidIterator create_target_iterator(const GPtrArray *target_array, map_target_function map_target, complete_target_mapping_function complete_target_mapping, void *data)
{
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)g_malloc(sizeof(TargetIteratorData));
    
    init_model_iterator_data(&target_iterator_data->model_iterator_data, target_array->len);
    target_iterator_data->target_array = target_array;
    target_iterator_data->map_target = map_target;
    target_iterator_data->complete_target_mapping = complete_target_mapping;
    target_iterator_data->data = data;
    
    return procreact_initialize_pid_iterator(has_next_target, next_target_process, procreact_retrieve_boolean, complete_target_process, target_iterator_data);
}

void destroy_target_iterator(ProcReact_PidIterator *iterator)
{
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)iterator->data;
    destroy_model_iterator_data(&target_iterator_data->model_iterator_data);
    g_free(target_iterator_data);
}

int target_iterator_has_succeeded(const ProcReact_PidIterator *iterator)
{
    TargetIteratorData *target_iterator_data = (TargetIteratorData*)iterator->data;
    return target_iterator_data->model_iterator_data.success;
}
