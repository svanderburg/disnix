/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2019  Sander van der Burg
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

#include "manifest.h"
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <targetstable.h>
#include "profilemappingtable.h"
#include "manifestservicestable.h"
#include "servicemappingarray.h"
#include "snapshotmappingarray.h"
#include <libxml/parser.h>
#include <nixxml-print-nix.h>
#include <nixxml-print-xml.h>
#include <nixxml-ghashtable.h>

#define BUFFER_SIZE 1024

static int check_version(xmlNodePtr element)
{
    xmlAttr *property = element->properties;

    while(property != NULL)
    {
        if(xmlStrcmp(property->name, (xmlChar*) "version") == 0 && xmlStrcmp(property->children->content, (xmlChar*) "2") == 0)
            return TRUE;

        property = property->next;
    }

    return FALSE;
}

static Manifest *parse_manifest(xmlNodePtr element, const unsigned int flags, const gchar *container_filter, const gchar *component_filter, void *userdata)
{
    if(check_version(element))
    {
        Manifest *manifest = (Manifest*)g_malloc0(sizeof(Manifest));
        xmlNodePtr child_element = element->children;

        while(child_element != NULL)
        {
            if((flags & MANIFEST_PROFILES_FLAG) && xmlStrcmp(child_element->name, (xmlChar*) "profiles") == 0)
                manifest->profile_mapping_table = parse_profile_mapping_table(child_element, userdata);
            else if(((flags & MANIFEST_SERVICE_MAPPINGS_FLAG) || (flags & MANIFEST_SNAPSHOT_MAPPINGS_FLAG)) && xmlStrcmp(child_element->name, (xmlChar*) "services") == 0)
                manifest->services_table = parse_services_table(child_element, userdata);
            else if((flags & MANIFEST_SERVICE_MAPPINGS_FLAG) && xmlStrcmp(child_element->name, (xmlChar*) "serviceMappings") == 0)
                manifest->service_mapping_array = parse_service_mapping_array(child_element, userdata);
            else if((flags & MANIFEST_SNAPSHOT_MAPPINGS_FLAG) && xmlStrcmp(child_element->name, (xmlChar*) "snapshotMappings") == 0)
                manifest->snapshot_mapping_array = parse_snapshot_mapping_array(child_element, container_filter, component_filter, userdata);
            else if((flags & MANIFEST_INFRASTRUCTURE_FLAG) && xmlStrcmp(child_element->name, (xmlChar*) "infrastructure") == 0)
                manifest->targets_table = parse_targets_table(child_element, userdata);

            child_element = child_element->next;
        }

        /* Set default values */
        if(manifest != NULL)
        {
            if(manifest->profile_mapping_table == NULL)
                manifest->profile_mapping_table = NixXML_create_g_hash_table();
            if(manifest->services_table == NULL)
                manifest->services_table = NixXML_create_g_hash_table();
            if(manifest->service_mapping_array == NULL)
                manifest->service_mapping_array = g_ptr_array_new();
            if(manifest->snapshot_mapping_array == NULL)
                manifest->snapshot_mapping_array = g_ptr_array_new();
            if(manifest->targets_table == NULL)
                manifest->targets_table = NixXML_create_g_hash_table();

            manifest->correct_version = TRUE;
        }

        return manifest;
    }
    else
    {
        Manifest *manifest = (Manifest*)g_malloc(sizeof(Manifest));
        manifest->profile_mapping_table = NixXML_create_g_hash_table();
        manifest->services_table = NixXML_create_g_hash_table();
        manifest->service_mapping_array = g_ptr_array_new();
        manifest->snapshot_mapping_array = g_ptr_array_new();
        manifest->targets_table = NixXML_create_g_hash_table();
        manifest->correct_version = FALSE;
        g_printerr("Disnix requires a manifest that uses the version 2 structure!\n");
        g_printerr("You can convert an old V1 version manifest file by running disnix-convert!\n");

        return manifest;
    }
}

Manifest *create_manifest(const gchar *manifest_file, const unsigned int flags, const gchar *container_filter, const gchar *component_filter)
{
    xmlDocPtr doc;
    xmlNodePtr node_root;
    Manifest *manifest;
    char buffer[BUFFER_SIZE];

    /* Parse the XML document */

    if((doc = xmlParseFile(manifest_file)) == NULL)
    {
        g_printerr("Error with parsing the manifest XML file!\n");
        xmlCleanupParser();
        return NULL;
    }

    /* Retrieve root element */
    node_root = xmlDocGetRootElement(doc);

    if(node_root == NULL)
    {
        g_printerr("The manifest XML file is empty!\n");
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }

    gethostname(buffer, BUFFER_SIZE);

    /* Parse manifest */
    manifest = parse_manifest(node_root, flags, container_filter, component_filter, buffer);

    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Return manifest */
    return manifest;
}

static int check_service_mapping_array_references(const GPtrArray *service_mapping_array, GHashTable *services_table, GHashTable *targets_table)
{
    unsigned int i;
    int status = TRUE;

    for(i = 0; i < service_mapping_array->len; i++)
    {
        ServiceMapping *mapping = g_ptr_array_index(service_mapping_array, i);

        if(!g_hash_table_contains(services_table, (gchar*)mapping->service))
        {
            g_printerr("Service mapping: %d contains an incorrect reference to service: %s\n", i, mapping->service);
            status = FALSE;
        }

        if(!g_hash_table_contains(targets_table, (gchar*)mapping->target))
        {
            g_printerr("Service mapping: %d contains an incorrect reference to target: %s\n", i, mapping->target);
            status = FALSE;
        }
    }

    return status;
}

static int check_snapshot_mapping_array_references(const GPtrArray *snapshot_mapping_array, GHashTable *services_table, GHashTable *targets_table)
{
    unsigned int i;
    int status = TRUE;

    for(i = 0; i < snapshot_mapping_array->len; i++)
    {
        SnapshotMapping *mapping = g_ptr_array_index(snapshot_mapping_array, i);

        if(!g_hash_table_contains(services_table, (gchar*)mapping->service))
        {
            g_printerr("Service mapping: %d contains an incorrect reference to service: %s\n", i, mapping->service);
            status = FALSE;
        }

        if(!g_hash_table_contains(targets_table, (gchar*)mapping->target))
        {
            g_printerr("Service mapping: %d contains an incorrect reference to target: %s\n", i, mapping->target);
            status = FALSE;
        }
    }

    return status;
}

int check_manifest(const Manifest *manifest)
{
    int status = TRUE;

    /* Check properties */

    if(!manifest->correct_version)
        status = FALSE;
    if(!check_profile_mapping_table(manifest->profile_mapping_table))
        status = FALSE;
    if(!check_services_table(manifest->services_table))
        status = FALSE;
    if(!check_service_mapping_array(manifest->service_mapping_array))
        status = FALSE;
    if(!check_snapshot_mapping_array(manifest->snapshot_mapping_array))
        status = FALSE;
    if(!check_targets_table(manifest->targets_table))
        status = FALSE;

    if(status)
    {
        /* Check the references of the mappings */

        if(!check_service_mapping_array_references(manifest->service_mapping_array, manifest->services_table, manifest->targets_table))
            status = FALSE;

        if(!check_snapshot_mapping_array_references(manifest->snapshot_mapping_array, manifest->services_table, manifest->targets_table))
            status = FALSE;
    }

    return status;
}

void delete_manifest(Manifest *manifest)
{
    if(manifest != NULL)
    {
        delete_profile_mapping_table(manifest->profile_mapping_table);
        delete_services_table(manifest->services_table);
        delete_service_mapping_array(manifest->service_mapping_array);
        delete_snapshot_mapping_array(manifest->snapshot_mapping_array);
        delete_targets_table(manifest->targets_table);
        g_free(manifest);
    }
}

int compare_manifests(const Manifest *manifest1, const Manifest *manifest2)
{
    return (compare_profile_mapping_tables(manifest1->profile_mapping_table, manifest2->profile_mapping_table)
      && compare_services_tables(manifest1->services_table, manifest2->services_table)
      && compare_service_mapping_arrays(manifest1->service_mapping_array, manifest2->service_mapping_array)
      && compare_snapshot_mapping_arrays(manifest1->snapshot_mapping_array, manifest2->snapshot_mapping_array)
      && compare_targets_tables(manifest1->targets_table, manifest2->targets_table));
}

static void print_manifest_attributes_nix(FILE *file, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    const Manifest *manifest = (const Manifest*)value;

    NixXML_print_attribute_nix(file, "profiles", manifest->profile_mapping_table, indent_level, userdata, (NixXML_PrintValueFunc)print_profile_mapping_table_nix);
    NixXML_print_attribute_nix(file, "services", manifest->services_table, indent_level, userdata, (NixXML_PrintValueFunc)print_services_table_nix);
    NixXML_print_attribute_nix(file, "serviceMappings", manifest->service_mapping_array, indent_level, userdata, (NixXML_PrintValueFunc)print_service_mapping_array_nix);
    NixXML_print_attribute_nix(file, "snapshotMappings", manifest->snapshot_mapping_array, indent_level, userdata, (NixXML_PrintValueFunc)print_snapshot_mapping_array_nix);
    NixXML_print_attribute_nix(file, "infrastructure", manifest->targets_table, indent_level, userdata, (NixXML_PrintValueFunc)print_targets_table_nix);
}

void print_manifest_nix(FILE *file, const Manifest *manifest, const int indent_level, void *userdata)
{
    NixXML_print_attrset_nix(file, manifest, indent_level, userdata, print_manifest_attributes_nix, NULL);
}

static void print_manifest_attributes_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    const Manifest *manifest = (const Manifest*)value;

    NixXML_print_simple_attribute_xml(file, "profiles", manifest->profile_mapping_table, indent_level, NULL, userdata, (NixXML_PrintXMLValueFunc)print_profile_mapping_table_xml);
    NixXML_print_simple_attribute_xml(file, "services", manifest->services_table, indent_level, NULL, userdata, (NixXML_PrintXMLValueFunc)print_services_table_xml);
    NixXML_print_simple_attribute_xml(file, "serviceMappings", manifest->service_mapping_array, indent_level, NULL, userdata, (NixXML_PrintXMLValueFunc)print_service_mapping_array_xml);
    NixXML_print_simple_attribute_xml(file, "snapshotMappings", manifest->snapshot_mapping_array, indent_level, NULL, userdata, (NixXML_PrintXMLValueFunc)print_snapshot_mapping_array_xml);
    NixXML_print_simple_attribute_xml(file, "infrastructure", manifest->targets_table, indent_level, NULL, userdata, (NixXML_PrintXMLValueFunc)print_targets_table_xml);
}

void print_manifest_xml(FILE *file, const Manifest *manifest, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_simple_attrset_xml(file, manifest, indent_level, NULL, userdata, print_manifest_attributes_xml, NULL);
}

gchar *determine_previous_manifest_file(const gchar *coordinator_profile_path, const gchar *profile)
{
    gchar *old_manifest_file;
    FILE *file;
    char *username = (getpwuid(geteuid()))->pw_name; /* Get current username */

    if(coordinator_profile_path == NULL)
        old_manifest_file = g_strconcat(LOCALSTATEDIR "/nix/profiles/per-user/", username, "/disnix-coordinator/", profile, NULL);
    else
        old_manifest_file = g_strconcat(coordinator_profile_path, "/", profile, NULL);

    /* Try to open file => if it succeeds we have a previous configuration */
    file = fopen(old_manifest_file, "r");

    if(file == NULL)
    {
        g_free(old_manifest_file);
        old_manifest_file = NULL;
    }
    else
        fclose(file);

    return old_manifest_file;
}

Manifest *open_provided_or_previous_manifest_file(const gchar *manifest_file, const gchar *coordinator_profile_path, gchar *profile, const unsigned int flags, const gchar *container, const gchar *component)
{
    if(manifest_file == NULL)
    {
        /* If no manifest file has been provided, try opening the last deployed one */
        gchar *old_manifest_file = determine_previous_manifest_file(coordinator_profile_path, profile);

        if(old_manifest_file == NULL)
            return NULL; /* There is no previously deployed manifest */
        else
        {
            /* Open the previously deployed manifest */
            Manifest *manifest;
            g_printerr("[coordinator]: Using previous manifest: %s\n", old_manifest_file);
            manifest = create_manifest(old_manifest_file, flags, container, component);
            g_free(old_manifest_file);
            return manifest;
        }
    }
    else
        return create_manifest(manifest_file, flags, container, component); /* Open the provided manifest file */
}

gchar *determine_manifest_to_open(const gchar *old_manifest, const gchar *coordinator_profile_path, gchar *profile)
{
    if(old_manifest == NULL)
        return determine_previous_manifest_file(coordinator_profile_path, profile);
    else
        return g_strdup(old_manifest);
}

Manifest *open_previous_manifest(const gchar *manifest_file, const unsigned int flags, const gchar *container_filter, const gchar *component_filter)
{
    if(manifest_file == NULL)
        return NULL;
    else
    {
        g_printerr("[coordinator]: Using previous manifest: %s\n", manifest_file);
        return create_manifest(manifest_file, flags, container_filter, component_filter);
    }
}
