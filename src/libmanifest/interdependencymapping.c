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

#include "interdependencymapping.h"
#include <nixxml-parse.h>
#include <nixxml-print-nix.h>
#include <nixxml-print-xml.h>

gint compare_interdependency_mappings(const InterDependencyMapping **l, const InterDependencyMapping **r)
{
    const InterDependencyMapping *left = *l;
    const InterDependencyMapping *right = *r;

    gint status = xmlStrcmp(left->target, right->target);

    if(status == 0)
    {
        status = xmlStrcmp(left->container, right->container);

        if(status == 0)
            return xmlStrcmp(left->service, right->service);
        else
            return status;
    }
    else
        return status;
}

static void *create_interdependency_mapping_from_element(xmlNodePtr element, void *userdata)
{
    return g_malloc0(sizeof(InterDependencyMapping));
}

void insert_interdependency_mapping_attributes(void *table, const xmlChar *key, void *value, void *userdata)
{
    InterDependencyMapping *mapping = (InterDependencyMapping*)table;

    if(xmlStrcmp(key, (xmlChar*) "service") == 0)
        mapping->service = value;
    else if(xmlStrcmp(key, (xmlChar*) "target") == 0)
        mapping->target = value;
    else if(xmlStrcmp(key, (xmlChar*) "container") == 0)
        mapping->container = value;
    else
        xmlFree(value);
}

void *parse_interdependency_mapping(xmlNodePtr element, void *userdata)
{
    InterDependencyMapping *mapping = NixXML_parse_simple_attrset(element, userdata, create_interdependency_mapping_from_element, NixXML_parse_value, insert_interdependency_mapping_attributes);

    /* Set default values */
    if(mapping->target == NULL)
        mapping->target = xmlStrdup((xmlChar*)userdata);

    return mapping;
}

void delete_interdependency_mapping(InterDependencyMapping *mapping)
{
    if(mapping != NULL)
    {
        xmlFree(mapping->service);
        xmlFree(mapping->container);
        xmlFree(mapping->target);
        g_free(mapping);
    }
}

int check_interdependency_mapping(const InterDependencyMapping *mapping)
{
    int status = TRUE;

    if(mapping->service == NULL)
    {
        g_printerr("mapping.service is not set!\n");
        status = FALSE;
    }
    else if(mapping->container == NULL)
    {
        g_printerr("mapping.container is not set!\n");
        status = FALSE;
    }
    else if(mapping->target == NULL)
    {
        g_printerr("mapping.target is not set!\n");
        status = FALSE;
    }

    return status;
}

static void print_interdependency_mapping_attributes_nix(FILE *file, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    InterDependencyMapping *mapping = (InterDependencyMapping*)value;

    NixXML_print_attribute_nix(file, "service", mapping->service, indent_level, userdata, NixXML_print_string_nix);
    NixXML_print_attribute_nix(file, "container", mapping->container, indent_level, userdata, NixXML_print_string_nix);
    NixXML_print_attribute_nix(file, "target", mapping->target, indent_level, userdata, NixXML_print_string_nix);
}

void print_interdependency_mapping_nix(FILE *file, const InterDependencyMapping *mapping, const int indent_level, void *userdata)
{
    NixXML_print_attrset_nix(file, mapping, indent_level, userdata, print_interdependency_mapping_attributes_nix, NULL);
}

static void print_interdependency_mapping_attributes_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    InterDependencyMapping *mapping = (InterDependencyMapping*)value;

    NixXML_print_simple_attribute_xml(file, "service", mapping->service, indent_level, NULL, userdata, NixXML_print_string_xml);
    NixXML_print_simple_attribute_xml(file, "container", mapping->container, indent_level, NULL, userdata, NixXML_print_string_xml);
    NixXML_print_simple_attribute_xml(file, "target", mapping->target, indent_level, NULL, userdata, NixXML_print_string_xml);
}

void print_interdependency_mapping_xml(FILE *file, const InterDependencyMapping *mapping, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_simple_attrset_xml(file, mapping, indent_level, NULL, userdata, print_interdependency_mapping_attributes_xml, NULL);
}
