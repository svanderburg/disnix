/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2016  Sander van der Burg
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

Manifest *create_manifest(const gchar *manifest_file, const unsigned int flags, const gchar *container_filter, const gchar *component_filter)
{
    Manifest *manifest = (Manifest*)g_malloc(sizeof(Manifest));
    manifest->distribution_array = NULL;
    manifest->activation_array = NULL;
    manifest->snapshots_array = NULL;
    manifest->target_array = NULL;
    
    if(flags & MANIFEST_DISTRIBUTION_FLAG)
    {
        manifest->distribution_array = generate_distribution_array(manifest_file);
        
        if(manifest->distribution_array == NULL)
        {
            delete_manifest(manifest);
            return NULL;
        }
    }
    
    if(flags & MANIFEST_ACTIVATION_FLAG)
    {
        manifest->activation_array = create_activation_array(manifest_file);
        
        if(manifest->activation_array == NULL)
        {
            delete_manifest(manifest);
            return NULL;
        }
    }
    
    if(flags & MANIFEST_SNAPSHOT_FLAG)
    {
        manifest->snapshots_array = create_snapshots_array(manifest_file, container_filter, component_filter);
        
        if(manifest->snapshots_array == NULL)
        {
            delete_manifest(manifest);
            return NULL;
        }
    }

    manifest->target_array = generate_target_array(manifest_file);
    
    if(manifest->target_array == NULL)
    {
        delete_manifest(manifest);
        return NULL;
    }
    else
        return manifest;
}

void delete_manifest(Manifest *manifest)
{
    if(manifest != NULL)
    {
        delete_distribution_array(manifest->distribution_array);
        delete_activation_array(manifest->activation_array);
        delete_snapshots_array(manifest->snapshots_array);
        delete_target_array(manifest->target_array);
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
            Manifest *manifest = create_manifest(old_manifest_file, flags, container, component);
            g_free(old_manifest_file);
            return manifest;
        }
    }
    else
        return create_manifest(manifest_file, flags, container, component); /* Open the provided manifest file */
}
