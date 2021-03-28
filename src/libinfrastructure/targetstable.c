/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2021  Sander van der Burg
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
#include <nixxml-ghashtable.h>
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

    /* Return the targets table */
    return targets_table;
}

static void augment_default_properties(GHashTable *targets_table, char *default_target_property, char *default_client_interface)
{
    if(default_target_property != NULL || default_client_interface != NULL)
    {
        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init(&iter, targets_table);
        while(g_hash_table_iter_next(&iter, &key, &value))
        {
            Target *target = (Target*)value;

            if(target->target_property == NULL)
                target->target_property = xmlStrdup((xmlChar*)default_target_property);

            if(target->client_interface == NULL)
                target->client_interface = xmlStrdup((xmlChar*)default_client_interface);
        }
    }
}

GHashTable *create_targets_table_from_xml(const char *infrastructure_xml, char *default_target_property, char *default_client_interface)
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

    /* Augment default properties where needed */
    augment_default_properties(targets_table, default_target_property, default_client_interface);

    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Return the targets table */
    return targets_table;
}

GHashTable *create_targets_table(gchar *infrastructure_expr, const NixXML_bool xml, char *default_target_property, char *default_client_interface)
{
    if(xml)
        return create_targets_table_from_xml(infrastructure_expr, default_target_property, default_client_interface);
    else
        return create_targets_table_from_nix(infrastructure_expr, default_target_property, default_client_interface);
}

void delete_targets_table(GHashTable *targets_table)
{
    NixXML_delete_g_hash_table(targets_table, (NixXML_DeleteGHashTableValueFunc)delete_target);
}

NixXML_bool check_targets_table(GHashTable *targets_table)
{
    return NixXML_check_g_hash_table(targets_table, (NixXML_CheckGHashTableValueFunc)check_target);
}

NixXML_bool compare_targets_tables(GHashTable *targets_table1, GHashTable *targets_table2)
{
    return NixXML_compare_g_hash_tables(targets_table1, targets_table2, (NixXML_CompareGHashTableValueFunc)compare_targets);
}

void print_targets_table_nix(FILE *file, GHashTable *targets_table, const int indent_level, void *userdata)
{
    NixXML_print_g_hash_table_nix(file, targets_table, indent_level, userdata, (NixXML_PrintValueFunc)print_target_nix);
}

void print_targets_table_xml(FILE *file, GHashTable *targets_table, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_g_hash_table_verbose_xml(file, targets_table, "target", "name", indent_level, NULL, userdata, (NixXML_PrintXMLValueFunc)print_target_xml);
}
