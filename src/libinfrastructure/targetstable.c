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

#include "targetstable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libxslt/xslt.h>
#include <libxslt/transform.h>
#include <nixxml-print-nix.h>
#include <nixxml-ghashtable.h>
#include <nixxml-gptrarray.h>
#include "package-management.h"

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
    return NixXML_parse_g_hash_table_verbose(element, "property", "name", userdata, NixXML_parse_value);
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
        target->properties_table = NixXML_parse_g_hash_table_verbose(element, "property", "name", userdata, NixXML_parse_value);
    else if(xmlStrcmp(key, (xmlChar*) "containers") == 0)
        target->containers_table = NixXML_parse_g_hash_table_verbose(element, "container", "name", userdata, parse_container);
}

static gpointer parse_target(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_simple_heterogeneous_attrset(element, userdata, create_target, parse_and_insert_target_attributes);
}

GHashTable *parse_targets_table(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_g_hash_table_verbose(element, "target", "name", userdata, parse_target);
}

GHashTable *create_targets_table_from_doc(xmlDocPtr doc)
{
    xmlNodePtr node_root;
    GHashTable *targets_table;

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

    /* Parse targets table */
    targets_table = parse_targets_table(node_root, NULL);

    /* Return targets table */
    return targets_table;
}

GHashTable *create_targets_table_from_nix(char *infrastructure_expr, char *default_target_property, char *default_client_interface)
{
    /* Declarations */
    xmlDocPtr doc;
    GHashTable *targets_table;

    /* Open the XML output of nix-instantiate */
    char *infrastructureXML = pkgmgmt_normalize_infrastructure_sync(infrastructure_expr, default_target_property, default_client_interface);

    if(infrastructureXML == NULL)
    {
        g_printerr("Error opening infrastructure XML file!\n");
        return NULL;
    }

    /* Parse the infrastructure XML file */
    if((doc = create_infrastructure_doc(infrastructureXML)) == NULL)
        targets_table = NULL;
    else
    {
        targets_table = create_targets_table_from_doc(doc); /* Create a target array from the XML document */
        xmlFreeDoc(doc);
    }

    /* Cleanup */
    free(infrastructureXML);
    xmlCleanupParser();

    /* Return the targets table*/
    return targets_table;
}

GHashTable *create_targets_table_from_xml(const char *infrastructure_xml)
{
    /* Declarations */
    xmlDocPtr doc;
    GHashTable *targets_table;

    /* Open the XML file */
    if((doc = xmlParseFile(infrastructure_xml)) == NULL)
    {
        g_printerr("Error with parsing the manifest XML file!\n");
        xmlCleanupParser();
        return NULL;
    }

    /* Create a targets table from the XML document */
    targets_table = create_targets_table_from_doc(doc);

    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Return the targets table */
    return targets_table;
}

GHashTable *create_targets_table(gchar *infrastructure_expr, const int xml, char *default_target_property, char *default_client_interface)
{
    if(xml)
        return create_targets_table_from_xml(infrastructure_expr);
    else
        return create_targets_table_from_nix(infrastructure_expr, default_target_property, default_client_interface);
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

void delete_targets_table(GHashTable *targets_table)
{
    if(targets_table != NULL)
    {
        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init(&iter, targets_table);
        while (g_hash_table_iter_next(&iter, &key, &value))
        {
            Target *target = (Target*)value;
            delete_target(target);
        }

        g_hash_table_destroy(targets_table);
    }
}

int check_targets_table(GHashTable *targets_table)
{
    if(targets_table == NULL)
        return TRUE;
    else
    {
        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init(&iter, targets_table);
        while (g_hash_table_iter_next(&iter, &key, &value))
        {
            Target *target = (Target*)value;

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

static void print_properties_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    NixXML_print_g_hash_table_nix(file, (GHashTable*)value, indent_level, userdata, NixXML_print_string_nix);
}

static void print_container_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    NixXML_print_g_hash_table_nix(file, (GHashTable*)value, indent_level, userdata, NixXML_print_string_nix);
}

static void print_containers_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    NixXML_print_g_hash_table_nix(file, (GHashTable*)value, indent_level, userdata, print_container_nix);
}

static void print_target_attributes_nix(FILE *file, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    Target *target = (Target*)value;

    NixXML_print_attribute_nix(file, "properties", target->properties_table, indent_level, userdata, print_properties_nix);
    NixXML_print_attribute_nix(file, "containers", target->containers_table, indent_level, userdata, print_containers_nix);
    if(target->name != NULL)
        NixXML_print_attribute_nix(file, "name", target->name, indent_level, userdata, NixXML_print_string_nix);
    if(target->system != NULL)
        NixXML_print_attribute_nix(file, "system", target->system, indent_level, userdata, NixXML_print_string_nix);
    if(target->client_interface != NULL)
        NixXML_print_attribute_nix(file, "clientInterface", target->client_interface, indent_level, userdata, NixXML_print_string_nix);
    if(target->target_property != NULL)
        NixXML_print_attribute_nix(file, "targetProperty", target->target_property, indent_level, userdata, NixXML_print_string_nix);
    NixXML_print_attribute_nix(file, "numOfCores", &target->num_of_cores, indent_level, userdata, NixXML_print_int_nix);
}

static void print_target_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    NixXML_print_attrset_nix(file, value, indent_level, userdata, print_target_attributes_nix, NULL);
}

void print_targets_table_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    NixXML_print_g_hash_table_nix(file, (GHashTable*)value, indent_level, userdata, print_target_nix);
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
