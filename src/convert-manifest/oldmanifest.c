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

#include "oldmanifest.h"
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include "distributionmapping.h"
#include "activationmapping.h"
#include "oldsnapshotmapping.h"
#include <libxml/parser.h>

static int check_version(xmlNodePtr element)
{
    xmlAttr *property = element->properties;

    while(property != NULL)
    {
        if(xmlStrcmp(property->name, (xmlChar*) "version") == 0 && xmlStrcmp(property->children->content, (xmlChar*) "1") == 0)
            return TRUE;

        property = property->next;
    }

    return FALSE;
}


static OldManifest *parse_old_manifest(xmlNodePtr element)
{
    if(check_version(element))
    {
        OldManifest *manifest = (OldManifest*)g_malloc0(sizeof(OldManifest));
        xmlNodePtr element_children = element->children;

        while(element_children != NULL)
        {
            if(xmlStrcmp(element_children->name, (xmlChar*) "distribution") == 0)
                manifest->distribution_array = parse_distribution(element_children);
            else if(xmlStrcmp(element_children->name, (xmlChar*) "activation") == 0)
                manifest->activation_array = parse_activation(element_children);
            else if(xmlStrcmp(element_children->name, (xmlChar*) "snapshots") == 0)
                manifest->snapshots_array = parse_old_snapshots(element_children);

            element_children = element_children->next;
        }

        return manifest;
    }
    else
    {
        g_printerr("Please provide a manifest file that uses the V1 structure!\n");
        return NULL;
    }
}

OldManifest *create_old_manifest(const gchar *manifest_file)
{
    xmlDocPtr doc;
    xmlNodePtr node_root;
    OldManifest *manifest;

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

    /* Parse manifest */
    manifest = parse_old_manifest(node_root);

    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Return manifest */
    return manifest;
}

int check_old_manifest(const OldManifest *manifest)
{
    return (check_distribution_array(manifest->distribution_array)
      && check_activation_array(manifest->activation_array)
      && check_old_snapshots_array(manifest->snapshots_array));
}

void delete_old_manifest(OldManifest *manifest)
{
    if(manifest != NULL)
    {
        delete_distribution_array(manifest->distribution_array);
        delete_activation_array(manifest->activation_array);
        delete_old_snapshots_array(manifest->snapshots_array);
        g_free(manifest);
    }
}

static gchar *determine_previous_manifest_file(const gchar *coordinator_profile_path, const gchar *profile)
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

OldManifest *open_provided_or_previous_old_manifest_file(const gchar *manifest_file, const gchar *coordinator_profile_path, gchar *profile)
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
            OldManifest *manifest;
            g_printerr("[coordinator]: Using previous manifest: %s\n", old_manifest_file);
            manifest = create_old_manifest(old_manifest_file);
            g_free(old_manifest_file);
            return manifest;
        }
    }
    else
        return create_old_manifest(manifest_file); /* Open the provided manifest file */
}
