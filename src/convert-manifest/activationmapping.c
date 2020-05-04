/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2020  Sander van der Burg
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
#include <stdlib.h>
#include <nixxml-gptrarray.h>

static gint compare_activation_mapping_keys(const ActivationMappingKey **l, const ActivationMappingKey **r)
{
    const ActivationMappingKey *left = *l;
    const ActivationMappingKey *right = *r;

    /* Compare the service keys */
    gint status = xmlStrcmp(left->key, right->key);

    if(status == 0)
    {
        status = xmlStrcmp(left->target, right->target); /* If services are equal then compare the targets */

        if(status == 0)
            return xmlStrcmp(left->container, right->container); /* If targets are equal then compare the containers */
        else
            return status;
    }
    else
        return status;
}

static gint compare_activation_mapping(const ActivationMapping **l, const ActivationMapping **r)
{
    return compare_activation_mapping_keys((const ActivationMappingKey **)l, (const ActivationMappingKey **)r);
}

static void *create_activation_mapping_key(xmlNodePtr element, void *userdata)
{
    return g_malloc0(sizeof(ActivationMappingKey));
}

static void insert_activation_mapping_key_attributes(void *table, const xmlChar *key, void *value, void *userdata)
{
    ActivationMappingKey *activation_mapping_key = (ActivationMappingKey*)table;

    if(xmlStrcmp(key, (xmlChar*) "key") == 0)
        activation_mapping_key->key = value;
    else if(xmlStrcmp(key, (xmlChar*) "target") == 0)
        activation_mapping_key->target = value;
    else if(xmlStrcmp(key, (xmlChar*) "container") == 0)
        activation_mapping_key->container = value;
    else
        xmlFree(value);
}

static gpointer parse_dependency_key(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_simple_attrset(element, userdata, create_activation_mapping_key, NixXML_parse_value, insert_activation_mapping_key_attributes);
}

static GPtrArray *parse_inter_dependencies(xmlNodePtr element)
{
    GPtrArray *inter_dependency_array = NixXML_parse_g_ptr_array(element, "dependency", NULL, parse_dependency_key);

    /* Sort the dependency array */
    g_ptr_array_sort(inter_dependency_array, (GCompareFunc)compare_activation_mapping_keys);

    return inter_dependency_array;
}

static void *create_activation_mapping(xmlNodePtr element, void *userdata)
{
    return g_malloc0(sizeof(ActivationMapping));
}

static void parse_and_insert_activation_mapping_attributes(xmlNodePtr element, void *table, const xmlChar *key, void *userdata)
{
    ActivationMapping *mapping = (ActivationMapping*)table;

    if(xmlStrcmp(key, (xmlChar*) "key") == 0)
        mapping->key = NixXML_parse_value(element, userdata);
    else if(xmlStrcmp(key, (xmlChar*) "service") == 0)
        mapping->service = NixXML_parse_value(element, userdata);
    else if(xmlStrcmp(key, (xmlChar*) "name") == 0)
        mapping->name = NixXML_parse_value(element, userdata);
    else if(xmlStrcmp(key, (xmlChar*) "type") == 0)
        mapping->type = NixXML_parse_value(element, userdata);
    else if(xmlStrcmp(key, (xmlChar*) "target") == 0)
        mapping->target = NixXML_parse_value(element, userdata);
    else if(xmlStrcmp(key, (xmlChar*) "container") == 0)
        mapping->container = NixXML_parse_value(element, userdata);
    else if(xmlStrcmp(key, (xmlChar*) "dependsOn") == 0)
        mapping->depends_on = parse_inter_dependencies(element);
    else if(xmlStrcmp(key, (xmlChar*) "connectsTo") == 0)
        mapping->connects_to = parse_inter_dependencies(element);
}

static gpointer parse_activation_mapping(xmlNodePtr element, void *userdata)
{
    ActivationMapping *mapping = NixXML_parse_simple_heterogeneous_attrset(element, userdata, create_activation_mapping, parse_and_insert_activation_mapping_attributes);

    /* If no properties or containers are specified, compose empty arrays */
    if(mapping->depends_on == NULL)
        mapping->depends_on = g_ptr_array_new();

    if(mapping->connects_to == NULL)
        mapping->connects_to = g_ptr_array_new();

    return mapping;
}

GPtrArray *parse_activation(xmlNodePtr element)
{
    GPtrArray *activation_array = NixXML_parse_g_ptr_array(element, "mapping", NULL, parse_activation_mapping);

    /* Sort the activation array */
    g_ptr_array_sort(activation_array, (GCompareFunc)compare_activation_mapping);

    return activation_array;
}

static void delete_inter_dependency_array(GPtrArray *inter_dependency_array)
{
    if(inter_dependency_array != NULL)
    {
        unsigned int i;

        for(i = 0; i < inter_dependency_array->len; i++)
        {
            ActivationMappingKey *dependency = g_ptr_array_index(inter_dependency_array, i);

            xmlFree(dependency->key);
            xmlFree(dependency->target);
            xmlFree(dependency->container);
            g_free(dependency);
        }

        g_ptr_array_free(inter_dependency_array, TRUE);
    }
}

void delete_activation_array(GPtrArray *activation_array)
{
    if(activation_array != NULL)
    {
        unsigned int i;

        for(i = 0; i < activation_array->len; i++)
        {
            ActivationMapping *mapping = g_ptr_array_index(activation_array, i);

            xmlFree(mapping->key);
            xmlFree(mapping->target);
            xmlFree(mapping->container);
            xmlFree(mapping->service);
            xmlFree(mapping->name);
            xmlFree(mapping->type);
            delete_inter_dependency_array(mapping->depends_on);
            delete_inter_dependency_array(mapping->connects_to);

            g_free(mapping);
        }

        g_ptr_array_free(activation_array, TRUE);
    }
}

int check_activation_array(const GPtrArray *activation_array)
{
    if(activation_array == NULL)
        return TRUE;
    else
    {
        unsigned int i;

        for(i = 0; i < activation_array->len; i++)
        {
            ActivationMapping *mapping = g_ptr_array_index(activation_array, i);

            if(mapping->key == NULL || mapping->target == NULL || mapping->container == NULL || mapping->service == NULL || mapping->name == NULL || mapping->type == NULL)
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
