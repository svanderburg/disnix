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

#include "profiles.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#include <distributionmapping.h>
#include <targets.h>
#include <client-interface.h>
#include <package-management.h>

static int set_target_profiles(const GPtrArray *distribution_array, const GPtrArray *target_array, gchar *profile)
{
    unsigned int i;
    int exit_status = 0, running_processes = 0;
    
    for(i = 0; i < distribution_array->len; i++)
    {
        pid_t pid;
        DistributionItem *item = g_ptr_array_index(distribution_array, i);
        Target *target = find_target(target_array, item->target);
        
        g_print("[target: %s]: Setting Disnix profile: %s\n", item->target, item->profile);
        
        pid = exec_set(target->client_interface, item->target, profile, item->profile);
        
        if(pid == -1)
        {
            g_print("[target: %s]: Error forking nix-env --set process!\n", item->target);
            exit_status = -1;
        }
        else
            running_processes++;
    }
    
    /* Check statusses of the running processes */
    for(i = 0; i < running_processes; i++)
    {
        int status = wait_to_finish(0);

        /* If one of the processes fail, change the exit status */
        if(status != 0)
        {
            g_printerr("Cannot set profile!\n");
            exit_status = status;
        }
    }

    return exit_status;
}

static void cleanup(GPtrArray *target_array, GPtrArray *distribution_array)
{
    delete_target_array(target_array);
    delete_distribution_array(distribution_array);
}

int set_profiles(const gchar *manifest_file, const gchar *coordinator_profile_path, char *profile, const int no_coordinator_profile, const int no_target_profiles)
{
    GPtrArray *distribution_array = generate_distribution_array(manifest_file);
    GPtrArray *target_array = generate_target_array(manifest_file);
    int status;
    
    if(!no_target_profiles && (status = set_target_profiles(distribution_array, target_array, profile)) != 0)
    {
        cleanup(target_array, distribution_array);
        return status;
    }
    
    if(!no_coordinator_profile && (status = pkgmgmt_set_coordinator_profile(coordinator_profile_path, manifest_file, profile)) != 0)
    {
        cleanup(target_array, distribution_array);
        return status;
    }
    
    cleanup(target_array, distribution_array);
    return 0;
}
