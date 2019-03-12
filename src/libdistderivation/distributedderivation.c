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
#include "derivationmapping.h"
#include "interfaces.h"

#include <libxml/parser.h>

static DistributedDerivation *parse_distributed_derivation(xmlNodePtr element)
{
    DistributedDerivation *distributed_derivation = (DistributedDerivation*)g_malloc0(sizeof(DistributedDerivation));
    xmlNodePtr element_children = element->children;

    while(element_children != NULL)
    {
        if(xmlStrcmp(element_children->name, (xmlChar*) "build") == 0)
            distributed_derivation->derivation_array = parse_build(element_children);
        else if(xmlStrcmp(element_children->name, (xmlChar*) "interfaces") == 0)
            distributed_derivation->interface_array = parse_interfaces(element_children);

        element_children = element_children->next;
    }

    return distributed_derivation;
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
    distributed_derivation = parse_distributed_derivation(node_root);

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
        delete_derivation_array(distributed_derivation->derivation_array);
        delete_interface_array(distributed_derivation->interface_array);
        g_free(distributed_derivation);
    }
}
