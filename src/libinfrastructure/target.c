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

#include "target.h"
#include <nixxml-parse.h>
#include <nixxml-parse-generic.h>
#include <nixxml-print-nix.h>
#include <nixxml-print-xml.h>
#include <nixxml-print-generic-nix.h>
#include <nixxml-print-generic-xml.h>
#include <nixxml-ghashtable.h>
#include <nixxml-gptrarray.h>
#include <nixxml-glib.h>
#include "targetpropertiestable.h"
#include "containerstable.h"

static void *create_target_from_element(xmlNodePtr element, void *userdata)
{
    Target *target = g_malloc0(sizeof(Target));
    target->num_of_cores = 1;
    return target;
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
        target->properties_table = parse_target_properties_table(element, userdata);
    else if(xmlStrcmp(key, (xmlChar*) "containers") == 0)
        target->containers_table = parse_containers_table(element, userdata);
}

void *parse_target(xmlNodePtr element, void *userdata)
{
    Target *target = NixXML_parse_simple_heterogeneous_attrset(element, userdata, create_target_from_element, parse_and_insert_target_attributes);

    /* Set default values */
    if(target != NULL)
    {
        if(target->properties_table == NULL)
            target->properties_table = NixXML_create_g_hash_table();
        if(target->containers_table == NULL)
            target->containers_table = NixXML_create_g_hash_table();
    }

    return target;
}

void delete_target(Target *target)
{
    if(target != NULL)
    {
        delete_target_properties_table(target->properties_table);
        delete_containers_table(target->containers_table);

        xmlFree(target->system);
        xmlFree(target->client_interface);
        xmlFree(target->target_property);
        g_free(target);
    }
}

NixXML_bool check_target(const Target *target)
{
    NixXML_bool status = TRUE;

    /* Check properties */
    if(target->num_of_cores == 0)
    {
        g_printerr("target.numOfCores should be greater than 0\n");
        status = FALSE;
    }

    if(target->target_property == NULL)
    {
        g_printerr("target.targetProperty is unspecified!\n");
        status = FALSE;
    }

    if(target->client_interface == NULL)
    {
        g_printerr("target.clientInterface is unspecified!\n");
        status = FALSE;
    }

    if(status)
    {
        /* Check if target property is defined */

        if(!g_hash_table_contains(target->properties_table, target->target_property))
        {
            g_printerr("Target defines: '%s' as a target property, but it does not exists in the properties attribute set!\n", target->target_property);
            status = FALSE;
        }
    }

    return status;
}

NixXML_bool compare_targets(const Target *left, const Target *right)
{
    return (compare_target_properties_tables(left->properties_table, right->properties_table)
      && compare_container_tables(left->containers_table, right->containers_table)
      && (xmlStrcmp(left->system, right->system) == 0)
      && (xmlStrcmp(left->client_interface, right->client_interface) == 0)
      && (xmlStrcmp(left->target_property, right->target_property) == 0)
      && (left->num_of_cores == right->num_of_cores));
}

/* Nix printing infrastructure */

static void print_target_attributes_nix(FILE *file, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    Target *target = (Target*)value;

    NixXML_print_attribute_nix(file, "properties", target->properties_table, indent_level, userdata, (NixXML_PrintValueFunc)print_target_properties_table_nix);
    NixXML_print_attribute_nix(file, "containers", target->containers_table, indent_level, userdata, (NixXML_PrintValueFunc)print_containers_table_nix);
    if(target->system != NULL)
        NixXML_print_attribute_nix(file, "system", target->system, indent_level, userdata, NixXML_print_string_nix);
    if(target->client_interface != NULL)
        NixXML_print_attribute_nix(file, "clientInterface", target->client_interface, indent_level, userdata, NixXML_print_string_nix);
    if(target->target_property != NULL)
        NixXML_print_attribute_nix(file, "targetProperty", target->target_property, indent_level, userdata, NixXML_print_string_nix);
    NixXML_print_attribute_nix(file, "numOfCores", &target->num_of_cores, indent_level, userdata, NixXML_print_int_nix);
}

void print_target_nix(FILE *file, const Target *target, const int indent_level, void *userdata)
{
    NixXML_print_attrset_nix(file, target, indent_level, userdata, print_target_attributes_nix, NULL);
}

/* XML printing infrastructure */

static void print_target_attributes_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    Target *target = (Target*)value;

    NixXML_print_simple_attribute_xml(file, "properties", target->properties_table, indent_level, NULL, userdata, (NixXML_PrintXMLValueFunc)print_target_properties_table_xml);
    NixXML_print_simple_attribute_xml(file, "containers", target->containers_table, indent_level, NULL, userdata, (NixXML_PrintXMLValueFunc)print_containers_table_xml);
    if(target->system != NULL)
        NixXML_print_simple_attribute_xml(file, "system", target->system, indent_level, NULL, userdata, NixXML_print_string_xml);
    if(target->client_interface != NULL)
        NixXML_print_simple_attribute_xml(file, "clientInterface", target->client_interface, indent_level, NULL, userdata, NixXML_print_string_xml);
    if(target->target_property != NULL)
        NixXML_print_simple_attribute_xml(file, "targetProperty", target->target_property, indent_level, NULL, userdata, NixXML_print_string_xml);
    NixXML_print_simple_attribute_xml(file, "numOfCores", &target->num_of_cores, indent_level, NULL, userdata, NixXML_print_int_xml);
}

void print_target_xml(FILE *file, const Target *target, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_simple_attrset_xml(file, target, indent_level, NULL, userdata, print_target_attributes_xml, NULL);
}

gchar *find_target_property(const Target *target, const gchar *name)
{
    if(target->properties_table == NULL || name == NULL)
        return NULL;
    else
    {
        NixXML_Node *node = g_hash_table_lookup(target->properties_table, name);

        if(node->type == NIX_XML_TYPE_STRING || node->type == NIX_XML_TYPE_INT || node->type == NIX_XML_TYPE_FLOAT || node->type == NIX_XML_TYPE_BOOL)
            return (gchar*)node->value;
        else
            return NULL;
    }
}

gchar *find_target_key(const Target *target)
{
    return find_target_property(target, (gchar*)target->target_property);
}

static GHashTable *find_container(GHashTable *containers_table, const gchar *name)
{
    if(containers_table == NULL)
        return NULL;
    else
        return g_hash_table_lookup(containers_table, name);
}

xmlChar **generate_activation_arguments_for_target(const Target *target, const gchar *container_name)
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

NixXML_bool request_available_target_core(Target *target)
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
