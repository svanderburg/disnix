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

Manifest *create_manifest(const gchar *manifest_file, const gchar *container_filter, const gchar *component_filter)
{
    GPtrArray *distribution_array, *activation_array, *snapshots_array, *target_array;
    Manifest *manifest;
    
    distribution_array = generate_distribution_array(manifest_file);
    if(distribution_array == NULL)
        return NULL;
    
    activation_array = create_activation_array(manifest_file);
    if(activation_array == NULL)
    {
        delete_distribution_array(distribution_array);
        return NULL;
    }
    snapshots_array = create_snapshots_array(manifest_file, container_filter, component_filter);
    if(snapshots_array == NULL)
    {
        delete_distribution_array(distribution_array);
        delete_activation_array(activation_array);
        return NULL;
    }
    target_array = generate_target_array(manifest_file);
    if(target_array == NULL)
    {
        delete_distribution_array(distribution_array);
        delete_activation_array(activation_array);
        delete_snapshots_array(snapshots_array);
        return NULL;
    }
    
    manifest = (Manifest*)g_malloc(sizeof(Manifest));
    manifest->distribution_array = distribution_array;
    manifest->activation_array = activation_array;
    manifest->snapshots_array = snapshots_array;
    manifest->target_array = target_array;
    
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
