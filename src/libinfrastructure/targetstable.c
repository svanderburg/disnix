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
#include <nixxml-parse.h>
#include <nixxml-parse-generic.h>
#include <nixxml-print-nix.h>
#include <nixxml-print-xml.h>
#include <nixxml-print-generic-nix.h>
#include <nixxml-print-generic-xml.h>
#include <nixxml-ghashtable.h>
#include <nixxml-gptrarray.h>
#include <nixxml-glib.h>
#include "package-management.h"
#include "hashtable-util.h"

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

static void *generic_parse_expr(xmlNodePtr element, void *userdata)
{
    return NixXML_generic_parse_expr_glib(element, "type", "name", userdata);
}

static void *parse_property_table(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_g_hash_table_verbose(element, "property", "name", userdata, generic_parse_expr);
}

static void *create_target(xmlNodePtr element, void *userdata)
{
    return g_malloc0(sizeof(Target));
}

static void parse_and_insert_target_attributes(xmlNodePtr element, void *table, const xmlChar *key, void *userdata)
{
    Target *target = (Target*)table;

    if(xmlStrcmp(key, (xmlChar*) "system") == 0)
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
        target->properties_table = parse_property_table(element, userdata);
    else if(xmlStrcmp(key, (xmlChar*) "containers") == 0)
        target->containers_table = NixXML_parse_g_hash_table_verbose(element, "container", "name", userdata, parse_property_table);
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
        g_hash_table_foreach(properties_table, delete_xml_value, NULL);
        g_hash_table_destroy(properties_table);
    }
}

static void delete_properties_func(gpointer key, gpointer value, gpointer user_data)
{
    delete_properties_table((GHashTable*)value);
}

static void delete_containers_table(GHashTable *containers_table)
{
    if(containers_table != NULL)
    {
        g_hash_table_foreach(containers_table, delete_properties_func, NULL);
        g_hash_table_destroy(containers_table);
    }
}

static void delete_target(Target *target)
{
    delete_properties_table(target->properties_table);
    delete_containers_table(target->containers_table);

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

static int check_target(const Target *target)
{
    if(target->properties_table == NULL)
    {
        g_printerr("target.properties is not set!\n");
        return FALSE;
    }

    return TRUE;
}

int check_targets_table(GHashTable *targets_table)
{
    if(targets_table == NULL)
        return TRUE;
    else
        return check_hash_table(targets_table, (CheckFunction)check_target);
}

static int compare_container_tables(GHashTable *containers_table1, GHashTable *containers_table2)
{
    return compare_hash_tables(containers_table1, containers_table2, (int (*) (const gpointer left, const gpointer right)) compare_property_tables);
}

static int compare_targets(const gpointer left, const gpointer right)
{
    const Target *target1 = (const Target*)left;
    const Target *target2 = (const Target*)right;

    return (compare_property_tables(target1->properties_table, target2->properties_table)
      && compare_container_tables(target1->containers_table, target2->containers_table)
      && (xmlStrcmp(target1->system, target2->system) == 0)
      && (xmlStrcmp(target1->client_interface, target2->client_interface) == 0)
      && (xmlStrcmp(target1->target_property, target2->target_property) == 0)
      && (target1->num_of_cores == target2->num_of_cores));
}

int compare_targets_tables(GHashTable *targets_table1, GHashTable *targets_table2)
{
    return compare_hash_tables(targets_table1, targets_table2, compare_targets);
}

/* Nix printing infrastructure */

static void print_generic_expr_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    NixXML_print_generic_expr_glib_nix(file, (const NixXML_Node*)value, indent_level);
}

static void print_properties_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    NixXML_print_g_hash_table_nix(file, (GHashTable*)value, indent_level, userdata, print_generic_expr_nix);
}

static void print_containers_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    NixXML_print_g_hash_table_nix(file, (GHashTable*)value, indent_level, userdata, print_properties_nix);
}

static void print_target_attributes_nix(FILE *file, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    Target *target = (Target*)value;

    NixXML_print_attribute_nix(file, "properties", target->properties_table, indent_level, userdata, print_properties_nix);
    NixXML_print_attribute_nix(file, "containers", target->containers_table, indent_level, userdata, print_containers_nix);
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

/* XML printing infrastructure */

static void print_generic_expr_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_generic_expr_glib_verbose_xml(file, (const NixXML_Node*)value, indent_level, "property", "attr", "name", "list", "type");
}

static void print_properties_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_g_hash_table_verbose_xml(file, (GHashTable*)value, "property", "name", indent_level, NULL, userdata, print_generic_expr_xml);
}

static void print_containers_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_g_hash_table_verbose_xml(file, (GHashTable*)value, "container", "name", indent_level, NULL, userdata, print_properties_xml);
}

static void print_target_attributes_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    Target *target = (Target*)value;

    NixXML_print_simple_attribute_xml(file, "properties", target->properties_table, indent_level, NULL, userdata, print_properties_xml);
    NixXML_print_simple_attribute_xml(file, "containers", target->containers_table, indent_level, NULL, userdata, print_containers_xml);
    if(target->system != NULL)
        NixXML_print_simple_attribute_xml(file, "system", target->system, indent_level, NULL, userdata, NixXML_print_string_xml);
    if(target->client_interface != NULL)
        NixXML_print_simple_attribute_xml(file, "clientInterface", target->client_interface, indent_level, NULL, userdata, NixXML_print_string_xml);
    if(target->target_property != NULL)
        NixXML_print_simple_attribute_xml(file, "targetProperty", target->target_property, indent_level, NULL, userdata, NixXML_print_string_xml);
    NixXML_print_simple_attribute_xml(file, "numOfCores", &target->num_of_cores, indent_level, NULL, userdata, NixXML_print_int_xml);
}

static void print_target_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_simple_attrset_xml(file, value, indent_level, NULL, userdata, print_target_attributes_xml, NULL);
}

void print_targets_table_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_g_hash_table_verbose_xml(file, (GHashTable*)value, "target", "name", indent_level, NULL, userdata, print_target_xml);
}

gchar *find_target_property(const Target *target, const gchar *name)
{
    if(target->properties_table == NULL || name == NULL)
        return NULL;
    else
    {
        NixXML_Node *node = g_hash_table_lookup(target->properties_table, name);
        if(node->type == NIX_XML_TYPE_STRING)
            return (gchar*)node->value;
        else
            return NULL;
    }
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

xmlChar **generate_activation_arguments(const Target *target, const gchar *container_name)
{
    GHashTable *container_table = find_container(target->containers_table, container_name);

    if(container_table == NULL)
    {
        xmlChar **ret = (xmlChar**)malloc(sizeof(xmlChar*));
        ret[0] = NULL;
        return ret;
    }
    else
        return NixXML_generate_env_vars_generic_glib(container_table);
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
