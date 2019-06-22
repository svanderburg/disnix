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

#include "manifest.h"
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include "distributionmapping.h"
#include "activationmapping.h"
#include "snapshotmapping.h"
#include "targets.h"
#include <libxml/parser.h>

static Manifest *parse_manifest(xmlNodePtr element, const unsigned int flags, const gchar *container_filter, const gchar *component_filter)
{
    Manifest *manifest = (Manifest*)g_malloc0(sizeof(Manifest));
    xmlNodePtr element_children = element->children;

    while(element_children != NULL)
    {
        if((flags & MANIFEST_DISTRIBUTION_FLAG) && xmlStrcmp(element_children->name, (xmlChar*) "profiles") == 0)
            manifest->distribution_table = parse_distribution(element_children);
        else if((flags & MANIFEST_ACTIVATION_FLAG) && xmlStrcmp(element_children->name, (xmlChar*) "activation") == 0)
            manifest->activation_array = parse_activation(element_children);
        else if((flags & MANIFEST_SNAPSHOT_FLAG) && xmlStrcmp(element_children->name, (xmlChar*) "snapshots") == 0)
            manifest->snapshots_array = parse_snapshots(element_children, container_filter, component_filter);
        else if((flags & MANIFEST_TARGETS_FLAG) && xmlStrcmp(element_children->name, (xmlChar*) "targets") == 0)
            manifest->targets_table = parse_targets_table(element_children);

        element_children = element_children->next;
    }

    return manifest;
}

Manifest *create_manifest(const gchar *manifest_file, const unsigned int flags, const gchar *container_filter, const gchar *component_filter)
{
    xmlDocPtr doc;
    xmlNodePtr node_root;
    Manifest *manifest;

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
    manifest = parse_manifest(node_root, flags, container_filter, component_filter);

    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Return manifest */
    return manifest;
}

int check_manifest(const Manifest *manifest)
{
    return (check_distribution_table(manifest->distribution_table)
      && check_activation_array(manifest->activation_array)
      && check_snapshots_array(manifest->snapshots_array)
      && check_targets_table(manifest->targets_table));
}

void delete_manifest(Manifest *manifest)
{
    if(manifest != NULL)
    {
        delete_distribution_table(manifest->distribution_table);
        delete_activation_array(manifest->activation_array);
        delete_snapshots_array(manifest->snapshots_array);
        delete_targets_table(manifest->targets_table);
        g_free(manifest);
    }
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

Manifest *open_previous_manifest(const gchar *manifest_file, const unsigned int flags, const gchar *container, const gchar *component)
{
    if(manifest_file == NULL)
        return NULL;
    else
    {
        g_printerr("[coordinator]: Using previous manifest: %s\n", manifest_file);
        return create_manifest(manifest_file, flags, container, component);
    }
}
