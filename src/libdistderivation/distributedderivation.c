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

#include "distributedderivation.h"
#include "derivationmappingarray.h"
#include "interfacestable.h"
#include <libxml/parser.h>
#include <nixxml-parse.h>

static void *create_distributed_derivation_from_element(xmlNodePtr element, void *userdata)
{
    return g_malloc0(sizeof(DistributedDerivation));
}

static void parse_and_insert_distributed_derivation_attributes(xmlNodePtr element, void *table, const xmlChar *key, void *userdata)
{
    DistributedDerivation *distributed_derivation = (DistributedDerivation*)table;

    if(xmlStrcmp(element->name, (xmlChar*) "derivationMappings") == 0)
        distributed_derivation->derivation_mapping_array = parse_derivation_mapping_array(element, userdata);
    else if(xmlStrcmp(element->name, (xmlChar*) "interfaces") == 0)
        distributed_derivation->interfaces_table = parse_interfaces_table(element, userdata);
}

static DistributedDerivation *parse_distributed_derivation(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_simple_heterogeneous_attrset(element, userdata, create_distributed_derivation_from_element, parse_and_insert_distributed_derivation_attributes);
}

DistributedDerivation *create_distributed_derivation(const gchar *distributed_derivation_file)
{
    /* Declarations */
    xmlDocPtr doc;
    xmlNodePtr node_root;
    DistributedDerivation *distributed_derivation;

    /* Parse the XML document */

    if((doc = xmlParseFile(distributed_derivation_file)) == NULL)
    {
        g_printerr("Error with parsing the distributed derivation XML file!\n");
        xmlCleanupParser();
        return NULL;
    }

    /* Retrieve root element */
    node_root = xmlDocGetRootElement(doc);

    if(node_root == NULL)
    {
        g_printerr("The distributed derivation XML file is empty!\n");
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }

    /* Parse distributed derivation */
    distributed_derivation = parse_distributed_derivation(node_root, NULL);

    /* Set default values */
    if(distributed_derivation->derivation_mapping_array == NULL)
        distributed_derivation->derivation_mapping_array = g_ptr_array_new();
    else if(distributed_derivation->interfaces_table == NULL)
        distributed_derivation->interfaces_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Return distributed derivation */
    return distributed_derivation;
}

void delete_distributed_derivation(DistributedDerivation *distributed_derivation)
{
    if(distributed_derivation != NULL)
    {
        delete_derivation_mapping_array(distributed_derivation->derivation_mapping_array);
        delete_interfaces_table(distributed_derivation->interfaces_table);
        g_free(distributed_derivation);
    }
}

int check_distributed_derivation(const DistributedDerivation *distributed_derivation)
{
    int status = TRUE;

    if(!check_derivation_mapping_array(distributed_derivation->derivation_mapping_array))
        status = FALSE;

    if(!check_interfaces_table(distributed_derivation->interfaces_table))
        status = FALSE;

    return status;
}
