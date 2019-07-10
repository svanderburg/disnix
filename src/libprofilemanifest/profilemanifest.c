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

#include "profilemanifest.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <nixxml-parse.h>
#include <nixxml-print-nix.h>
#include <servicestable.h>
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
    return NixXML_parse_simple_heterogeneous_attrset(element, userdata, create_profile_manifest_from_element, parse_and_insert_profile_manifest_attributes);
}

ProfileManifest *create_profile_manifest_from_string(char *result)
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
    profile_manifest = parse_profile_manifest(node_root, NULL);

    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Return profile manifest */
    return profile_manifest;
}

ProfileManifest *create_profile_manifest_from_file(const gchar *profile_manifest_file)
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
    profile_manifest = parse_profile_manifest(node_root, NULL);

    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Return profile manifest */
    return profile_manifest;
}

ProfileManifest *create_profile_manifest_from_current_deployment(gchar *localstatedir, gchar *profile)
{
    ProfileManifest *profile_manifest;
    gchar *profile_manifest_file = g_strconcat(localstatedir, "/nix/profiles/disnix/", profile, "/manifest", NULL);

    if(access(profile_manifest_file, F_OK) == -1)
    {
        /*
         * If no manifest profile exists means we have an initial deployment.
         * Return an empty profile manifest instead of NULL to indicate that
           this is not a failure
         */
        profile_manifest = g_malloc0(sizeof(ProfileManifest));
        profile_manifest->service_mapping_array = g_ptr_array_new();
    }
    else
        profile_manifest = create_profile_manifest_from_file(profile_manifest_file);

    g_free(profile_manifest_file);
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
    gchar *profile_manifest_file = g_strconcat(localstatedir, "/nix/profiles/disnix/", profile, "/manifest", NULL);
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

static void print_profile_manifest_attributes_nix(FILE *file, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    ProfileManifest *profile_manifest = (ProfileManifest*)value;
    NixXML_print_attribute_nix(file, "services", profile_manifest->services_table, indent_level, userdata, print_services_table_nix);
    NixXML_print_attribute_nix(file, "serviceMappings", profile_manifest->service_mapping_array, indent_level, userdata, print_service_mapping_array_nix);
    NixXML_print_attribute_nix(file, "snapshotMappings", profile_manifest->snapshot_mapping_array, indent_level, userdata, print_snapshot_mapping_array_nix);
}

void print_profile_manifest_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    NixXML_print_attrset_nix(file, value, indent_level, userdata, print_profile_manifest_attributes_nix, NULL);
}
