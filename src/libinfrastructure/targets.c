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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libxslt/xslt.h>
#include <libxslt/transform.h>
#include <nixxml-ghashtable.h>
#include <nixxml-gptrarray.h>
#include "package-management.h"

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

static gpointer parse_container(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_g_hash_table(element, userdata, NixXML_parse_value);
}

static void *create_target(xmlNodePtr element, void *userdata)
{
    return g_malloc0(sizeof(Target));
}

static void parse_and_insert_target_attributes(xmlNodePtr element, void *table, const xmlChar *key, void *userdata)
{
    Target *target = (Target*)table;

    if(xmlStrcmp(key, (xmlChar*) "name") == 0)
        target->name = NixXML_parse_value(element, userdata);
    else if(xmlStrcmp(key, (xmlChar*) "system") == 0)
        target->system = NixXML_parse_value(element, userdata);
    else if(xmlStrcmp(key, (xmlChar*) "clientInterface") == 0)
        target->client_interface = NixXML_parse_value(element, userdata);
    else if(xmlStrcmp(key, (xmlChar*) "targetProperty") == 0)
        target->target_property = NixXML_parse_value(element, userdata);
    else if(xmlStrcmp(key, (xmlChar*) "numOfCores") == 0)
    {
        gchar *num_of_cores_str = NixXML_parse_value(element, userdata);

        if(num_of_cores_str != NULL)
        {
            target->num_of_cores = atoi((char*)num_of_cores_str);
            target->available_cores = target->num_of_cores;
            g_free(num_of_cores_str);
        }
    }
    else if(xmlStrcmp(key, (xmlChar*) "properties") == 0)
        target->properties_table = NixXML_parse_g_hash_table(element, userdata, NixXML_parse_value);
    else if(xmlStrcmp(key, (xmlChar*) "containers") == 0)
        target->containers_table = NixXML_parse_g_hash_table(element, userdata, parse_container);
}

static gpointer parse_target(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_simple_heterogeneous_attrset(element, userdata, create_target, parse_and_insert_target_attributes);
}

GPtrArray *parse_targets(xmlNodePtr element)
{
    GPtrArray *targets_array = NixXML_parse_g_ptr_array(element, "target", NULL, parse_target);
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
    GPtrArray *targets_array;

    /* Open the XML output of nix-instantiate */
    char *infrastructureXML = pkgmgmt_instantiate_sync(infrastructure_expr);

    if(infrastructureXML == NULL)
    {
        g_printerr("Error opening infrastructure XML file!\n");
        return NULL;
    }

    /* Parse the infrastructure XML file */
    if((doc = create_infrastructure_doc(infrastructureXML)) == NULL)
        targets_array = NULL;
    else
    {
        targets_array = create_target_array_from_doc(doc); /* Create a target array from the XML document */
        xmlFreeDoc(doc);
    }

    /* Cleanup */
    free(infrastructureXML);
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
            xmlFree(value);

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
            delete_properties_table(container_table);
        }

        g_hash_table_destroy(containers_table);
    }
}

static void delete_target(Target *target)
{
    delete_properties_table(target->properties_table);
    delete_containers_table(target->containers_table);

    xmlFree(target->name);
    xmlFree(target->system);
    xmlFree(target->client_interface);
    xmlFree(target->target_property);
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

static int compare_target_keys(const char *key, const Target **r)
{
    const Target *right = *r;
    gchar *right_target_property = find_target_key(right, NULL);

    return g_strcmp0(key, right_target_property);
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
    if(target->properties_table == NULL || name == NULL)
        return NULL;
    else
        return g_hash_table_lookup(target->properties_table, name);
}

gchar *find_target_key(const Target *target, const gchar *global_target_property)
{
    if(target->target_property == NULL)
        return find_target_property(target, global_target_property);
    else
        return find_target_property(target, (gchar*)target->target_property);
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
