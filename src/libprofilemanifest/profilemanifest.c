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

#include "profilemanifest.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <nixxml-parse.h>
#include <nixxml-print-nix.h>
#include <nixxml-print-xml.h>
#include <nixxml-ghashtable.h>
#include <manifestservicestable.h>
#include <servicemappingarray.h>
#include <snapshotmappingarray.h>

#define BUFFER_SIZE 1024

static void *create_profile_manifest_from_element(xmlNodePtr element, void *userdata)
{
    return g_malloc0(sizeof(ProfileManifest));
}

static void parse_and_insert_profile_manifest_attributes(xmlNodePtr element, void *table, const xmlChar *key, void *userdata)
{
    ProfileManifest *profile_manifest = (ProfileManifest*)table;

    if(xmlStrcmp(key, (xmlChar*) "services") == 0)
        profile_manifest->services_table = parse_services_table(element, userdata);
    else if(xmlStrcmp(key, (xmlChar*) "serviceMappings") == 0)
        profile_manifest->service_mapping_array = parse_service_mapping_array(element, userdata);
    else if(xmlStrcmp(key, (xmlChar*) "snapshotMappings") == 0)
        profile_manifest->snapshot_mapping_array = parse_snapshot_mapping_array(element, NULL, NULL, userdata);
}

static ProfileManifest *parse_profile_manifest(xmlNodePtr element, void *userdata)
{
    ProfileManifest *profile_manifest = NixXML_parse_simple_heterogeneous_attrset(element, userdata, create_profile_manifest_from_element, parse_and_insert_profile_manifest_attributes);

    /* Set default values */
    if(profile_manifest != NULL)
    {
        if(profile_manifest->services_table == NULL)
            profile_manifest->services_table = NixXML_create_g_hash_table();
        if(profile_manifest->service_mapping_array == NULL)
            profile_manifest->service_mapping_array = g_ptr_array_new();
        if(profile_manifest->snapshot_mapping_array == NULL)
            profile_manifest->snapshot_mapping_array = g_ptr_array_new();
    }

    return profile_manifest;
}

ProfileManifest *create_profile_manifest_from_string(char *result, gchar *default_target)
{
    xmlDocPtr doc;
    xmlNodePtr node_root;
    ProfileManifest *profile_manifest;

    if((doc = xmlParseDoc((xmlChar*)result)) == NULL)
    {
        g_printerr("Error with parsing the profile manifest XML file!\n");
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

    /* Parse manifest */
    profile_manifest = parse_profile_manifest(node_root, default_target);

    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Return profile manifest */
    return profile_manifest;
}

ProfileManifest *create_profile_manifest_from_file(const gchar *profile_manifest_file, gchar *default_target)
{
    xmlDocPtr doc;
    xmlNodePtr node_root;
    ProfileManifest *profile_manifest;

    /* Parse the XML document */

    if((doc = xmlParseFile(profile_manifest_file)) == NULL)
    {
        g_printerr("Error with parsing the profile manifest XML file!\n");
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

    /* Parse manifest */
    profile_manifest = parse_profile_manifest(node_root, default_target);

    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Return profile manifest */
    return profile_manifest;
}

static gchar *determine_profile_dir(gchar *localstatedir)
{
    if(getuid() == 0)
        return g_strconcat(localstatedir, "/nix/profiles/disnix", NULL);
    else
    {
        char *username = (getpwuid(geteuid()))->pw_name;
        return g_strconcat(localstatedir, "/nix/profiles/per-user/", username, "/disnix", NULL);
    }
}

ProfileManifest *create_profile_manifest_from_current_deployment(gchar *localstatedir, gchar *profile, gchar *default_target)
{
    ProfileManifest *profile_manifest;
    gchar *profile_dir = determine_profile_dir(localstatedir);
    gchar *profile_manifest_file = g_strconcat(profile_dir, "/", profile, "/profilemanifest.xml", NULL);

    if(access(profile_manifest_file, F_OK) == -1)
    {
        /*
         * If no manifest profile exists means we have an initial deployment.
         * Return an empty profile manifest instead of NULL to indicate that
         * this is not a failure
         */
        profile_manifest = g_malloc(sizeof(ProfileManifest));
        profile_manifest->services_table = NixXML_create_g_hash_table();
        profile_manifest->service_mapping_array = g_ptr_array_new();
        profile_manifest->snapshot_mapping_array = g_ptr_array_new();
    }
    else
        profile_manifest = create_profile_manifest_from_file(profile_manifest_file, default_target);

    g_free(profile_manifest_file);
    g_free(profile_dir);
    return profile_manifest;
}

void delete_profile_manifest(ProfileManifest *profile_manifest)
{
    if(profile_manifest != NULL)
    {
        delete_services_table(profile_manifest->services_table);
        delete_service_mapping_array(profile_manifest->service_mapping_array);
        delete_snapshot_mapping_array(profile_manifest->snapshot_mapping_array);
        g_free(profile_manifest);
    }
}

int check_profile_manifest(const ProfileManifest *profile_manifest)
{
    int status = TRUE;

    if(!check_services_table(profile_manifest->services_table))
        status = FALSE;

    if(!check_service_mapping_array(profile_manifest->service_mapping_array))
        status = FALSE;

    if(!check_snapshot_mapping_array(profile_manifest->snapshot_mapping_array))
        status = FALSE;

    return status;
}

static void print_profile_manifest_attributes_nix(FILE *file, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    ProfileManifest *profile_manifest = (ProfileManifest*)value;

    NixXML_print_attribute_nix(file, "services", profile_manifest->services_table, indent_level, userdata, (NixXML_PrintValueFunc)print_services_table_nix);
    NixXML_print_attribute_nix(file, "serviceMappings", profile_manifest->service_mapping_array, indent_level, userdata, (NixXML_PrintValueFunc)print_service_mapping_array_nix);
    NixXML_print_attribute_nix(file, "snapshotMappings", profile_manifest->snapshot_mapping_array, indent_level, userdata, (NixXML_PrintValueFunc)print_snapshot_mapping_array_nix);
}

void print_profile_manifest_nix(FILE *file, const ProfileManifest *profile_manifest, const int indent_level, void *userdata)
{
    NixXML_print_attrset_nix(file, profile_manifest, indent_level, userdata, print_profile_manifest_attributes_nix, NULL);
}

void print_profile_manifest_attributes_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    ProfileManifest *profile_manifest = (ProfileManifest*)value;

    NixXML_print_simple_attribute_xml(file, "services", profile_manifest->services_table, indent_level, type_property_name, userdata, (NixXML_PrintXMLValueFunc)print_services_table_xml);
    NixXML_print_simple_attribute_xml(file, "serviceMappings", profile_manifest->service_mapping_array, indent_level, type_property_name, userdata, (NixXML_PrintXMLValueFunc)print_service_mapping_array_xml);
    NixXML_print_simple_attribute_xml(file, "snapshotMappings", profile_manifest->snapshot_mapping_array, indent_level, type_property_name, userdata, (NixXML_PrintXMLValueFunc)print_snapshot_mapping_array_xml);
}

void print_profile_manifest_xml(FILE *file, const ProfileManifest *profile_manifest, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_simple_attrset_xml(file, profile_manifest, indent_level, type_property_name, userdata, print_profile_manifest_attributes_xml, NULL);
}

void print_services_in_profile_manifest(const ProfileManifest *profile_manifest)
{
    unsigned int i;

    for(i = 0; i < profile_manifest->service_mapping_array->len; i++)
    {
        ServiceMapping *mapping = g_ptr_array_index(profile_manifest->service_mapping_array, i);
        ManifestService *service = g_hash_table_lookup(profile_manifest->services_table, mapping->service);

        g_print("%s\n", service->pkg);
    }
}

static gint compare_service_mapping(gconstpointer l, gconstpointer r)
{
    const ServiceMapping *left = *((ServiceMapping **)l);
    const ServiceMapping *right = *((ServiceMapping **)r);

    int result = xmlStrcmp(left->container, right->container);

    if(result == 0)
        return xmlStrcmp(left->service, right->service);
    else
        return result;
}

void print_services_per_container_in_profile_manifest(ProfileManifest *profile_manifest)
{
    unsigned int i;
    xmlChar *last_container = (xmlChar*)"";

    g_ptr_array_sort(profile_manifest->service_mapping_array, compare_service_mapping);

    for(i = 0; i < profile_manifest->service_mapping_array->len; i++)
    {
        ServiceMapping *mapping = g_ptr_array_index(profile_manifest->service_mapping_array, i);
        ManifestService *service = g_hash_table_lookup(profile_manifest->services_table, mapping->service);

        if(xmlStrcmp(mapping->container, last_container) != 0)
        {
            last_container = mapping->container;
            g_print("  Container: %s\n", last_container);
        }

        g_print("    %s\n", service->pkg);
    }
}

void print_text_from_profile_manifest(gchar *localstatedir, gchar *profile, int fd)
{
    gchar *profile_manifest_file = g_strconcat(localstatedir, "/nix/profiles/disnix/", profile, "/profilemanifest.xml", NULL);
    int file_fd;

    if((file_fd = open(profile_manifest_file, O_RDONLY)) != -1)
    {
        ssize_t bytes_read;
        char buffer[BUFFER_SIZE];

        while((bytes_read = read(file_fd, buffer, BUFFER_SIZE)) > 0)
        {
            if(write(fd, buffer, bytes_read) != bytes_read)
                break;
        }

        close(file_fd);
    }

    g_free(profile_manifest_file);
}
