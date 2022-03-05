/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2022  Sander van der Burg
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

#include "snapshotmapping.h"
#include <nixxml-parse.h>
#include <nixxml-print-nix.h>
#include <nixxml-print-xml.h>

gint compare_snapshot_mapping_keys(const SnapshotMappingKey **l, const SnapshotMappingKey **r)
{
    const SnapshotMappingKey *left = *l;
    const SnapshotMappingKey *right = *r;

    gint status = xmlStrcmp(left->target, right->target);

    if(status == 0)
    {
        gint status = xmlStrcmp(left->container, right->container);

        if(status == 0)
            return xmlStrcmp(left->component, right->component);
        else
            return status;
    }
    else
        return status;
}

gint compare_snapshot_mapping(const SnapshotMapping **l, const SnapshotMapping **r)
{
    return compare_snapshot_mapping_keys((const SnapshotMappingKey **)l, (const SnapshotMappingKey **)r);
}

static void *create_snapshot_mapping_from_element(xmlNodePtr element, void *userdata)
{
    return g_malloc0(sizeof(SnapshotMapping));
}

static void insert_snapshot_mapping_attributes(void *table, const xmlChar *key, void *value, void *userdata)
{
    SnapshotMapping *mapping = (SnapshotMapping*)table;

    if(xmlStrcmp(key, (xmlChar*) "component") == 0)
        mapping->component = value;
    else if(xmlStrcmp(key, (xmlChar*) "container") == 0)
        mapping->container = value;
    else if(xmlStrcmp(key, (xmlChar*) "target") == 0)
        mapping->target = value;
    else if(xmlStrcmp(key, (xmlChar*) "service") == 0)
        mapping->service = value;
    else if(xmlStrcmp(key, (xmlChar*) "containerProvidedByService") == 0)
        mapping->container_provided_by_service = value;
    else
        xmlFree(value);
}

void *parse_snapshot_mapping(xmlNodePtr element, void *userdata)
{
    SnapshotMapping *mapping = NixXML_parse_simple_attrset(element, userdata, create_snapshot_mapping_from_element, NixXML_parse_value, insert_snapshot_mapping_attributes);

    /* Set default values */
    if(mapping->target == NULL)
        mapping->target = xmlStrdup((xmlChar*)userdata);

    return mapping;
}

void delete_snapshot_mapping(SnapshotMapping *mapping)
{
    if(mapping != NULL)
    {
        xmlFree(mapping->component);
        xmlFree(mapping->container);
        xmlFree(mapping->target);
        xmlFree(mapping->service);
        xmlFree(mapping->container_provided_by_service);
        g_free(mapping);
    }
}

NixXML_bool check_snapshot_mapping(const SnapshotMapping *mapping)
{
    NixXML_bool status = TRUE;

    if(mapping->component == NULL)
    {
        g_printerr("mapping.component is not set!\n");
        status = FALSE;
    }
    else if(mapping->container == NULL)
    {
        g_printerr("mapping.container is not set!\n");
        status = FALSE;
    }
    else if(mapping->service == NULL)
    {
        g_printerr("mapping.service is not set!\n");
        status = FALSE;
    }
    else if(mapping->target == NULL)
    {
        g_printerr("mapping.target is not set!\n");
        status = FALSE;
    }

    return status;
}

static void print_snapshot_mapping_attributes_nix(FILE *file, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    SnapshotMapping *mapping = (SnapshotMapping*)value;

    NixXML_print_attribute_nix(file, "service", mapping->service, indent_level, userdata, NixXML_print_string_nix);
    NixXML_print_attribute_nix(file, "component", mapping->component, indent_level, userdata, NixXML_print_string_nix);
    NixXML_print_attribute_nix(file, "container", mapping->container, indent_level, userdata, NixXML_print_string_nix);
    NixXML_print_attribute_nix(file, "target", mapping->target, indent_level, userdata, NixXML_print_string_nix);
    if(mapping->container_provided_by_service != NULL)
        NixXML_print_attribute_nix(file, "containerProvidedByService", mapping->container_provided_by_service, indent_level, userdata, NixXML_print_string_nix);
}

void print_snapshot_mapping_nix(FILE *file, const SnapshotMapping *mapping, const int indent_level, void *userdata)
{
    NixXML_print_attrset_nix(file, mapping, indent_level, userdata, print_snapshot_mapping_attributes_nix, NULL);
}

static void print_snapshot_mapping_attributes_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    SnapshotMapping *mapping = (SnapshotMapping*)value;

    NixXML_print_simple_attribute_xml(file, "service", mapping->service, indent_level, NULL, userdata, NixXML_print_string_xml);
    NixXML_print_simple_attribute_xml(file, "component", mapping->component, indent_level, NULL, userdata, NixXML_print_string_xml);
    NixXML_print_simple_attribute_xml(file, "container", mapping->container, indent_level, NULL, userdata, NixXML_print_string_xml);
    NixXML_print_simple_attribute_xml(file, "target", mapping->target, indent_level, NULL, userdata, NixXML_print_string_xml);
    if(mapping->container_provided_by_service != NULL)
        NixXML_print_simple_attribute_xml(file, "containerProvidedByService", mapping->container_provided_by_service, indent_level, NULL, userdata, NixXML_print_string_xml);
}

void print_snapshot_mapping_xml(FILE *file, const SnapshotMapping *mapping, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_simple_attrset_xml(file, mapping, indent_level, type_property_name, userdata, print_snapshot_mapping_attributes_xml, NULL);
}

int mapping_is_selected(const SnapshotMapping *mapping, const gchar *container_filter, const gchar *component_filter)
{
    return (container_filter == NULL || xmlStrcmp((const xmlChar*) container_filter, mapping->container) == 0) && (component_filter == NULL || xmlStrcmp((const xmlChar*) component_filter, mapping->component) == 0);
}
