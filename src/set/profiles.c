/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2017  Sander van der Burg
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

#include "profiles.h"

#include <manifest.h>
#include <distributionmapping.h>
#include <targets.h>
#include <client-interface.h>
#include <package-management.h>

static pid_t set_distribution_item(void *data, DistributionItem *item, Target *target)
{
    char *profile = (char*)data;
    g_print("[target: %s]: Setting Disnix profile: %s\n", item->target, item->profile);
    return exec_set(target->client_interface, item->target, profile, item->profile);
}

static void complete_set_distribution_item(void *data, DistributionItem *item, ProcReact_Status status, int result)
{
    if(status != PROCREACT_STATUS_OK || !result)
        g_printerr("[target: %s]: Cannot set Disnix profile: %s\n", item->target, item->profile);
}

static int set_target_profiles(const GPtrArray *distribution_array, const GPtrArray *target_array, gchar *profile)
{
    /* Iterate over the distribution mappings, limiting concurrency to the desired concurrent transfers and distribute them */
    int success;
    ProcReact_PidIterator iterator = create_distribution_iterator(distribution_array, target_array, set_distribution_item, complete_set_distribution_item, profile);
    procreact_fork_in_parallel_and_wait(&iterator);
    success = distribution_iterator_has_succeeded(&iterator);
    
    destroy_distribution_iterator(&iterator);
    
    return success;
}

int set_profiles(const gchar *manifest_file, const gchar *coordinator_profile_path, char *profile, const int no_coordinator_profile, const int no_target_profiles)
{
    Manifest *manifest = create_manifest(manifest_file, MANIFEST_DISTRIBUTION_FLAG, NULL, NULL);
    
    if(manifest == NULL)
    {
        g_printerr("[coordinator]: Error opening manifest file!\n");
        return 1;
    }
    else
    {
        int exit_status;
        
        if((no_target_profiles || set_target_profiles(manifest->distribution_array, manifest->target_array, profile)) /* First, attempt to set the target profiles */
          && (no_coordinator_profile || pkgmgmt_set_coordinator_profile(coordinator_profile_path, manifest_file, profile))) /* Then try to set the coordinator profile */
            exit_status = 0;
        else
            exit_status = 1;
        
        /* Cleanup */
        delete_manifest(manifest);

        /* Return exit status */
        return exit_status;
    }
}
