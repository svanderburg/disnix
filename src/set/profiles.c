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
#include <procreact_pid_iterator.h>

typedef struct
{
    unsigned int index;
    unsigned int length;
    const GPtrArray *distribution_array;
    const GPtrArray *target_array;
    gchar *profile;
    int success;
}
DistributionIteratorData;

static int has_next_distribution_item(void *data)
{
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)data;
    return distribution_iterator_data->index < distribution_iterator_data->length;
}

static pid_t next_set_process(void *data)
{
    pid_t pid;
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)data;
    
    DistributionItem *item = g_ptr_array_index(distribution_iterator_data->distribution_array, distribution_iterator_data->index);
    Target *target = find_target(distribution_iterator_data->target_array, item->target);
    
    g_print("[target: %s]: Setting Disnix profile: %s\n", item->target, item->profile);
    pid = exec_set(target->client_interface, item->target, distribution_iterator_data->profile, item->profile);
    distribution_iterator_data->index++;
    return pid;
}

static void complete_set_process(void *data, pid_t pid, ProcReact_Status status, int result)
{
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)data;
    
    if(status != PROCREACT_STATUS_OK || !result)
        distribution_iterator_data->success = FALSE;
}

static int set_target_profiles(const GPtrArray *distribution_array, const GPtrArray *target_array, gchar *profile)
{
    /* Iterate over the distribution mappings, limiting concurrency to the desired concurrent transfers and distribute them */
    DistributionIteratorData data = { 0, distribution_array->len, distribution_array, target_array, profile, TRUE };
    ProcReact_PidIterator iterator = procreact_initialize_pid_iterator(has_next_distribution_item, next_set_process, procreact_retrieve_boolean, complete_set_process, &data);
    procreact_fork_in_parallel_and_wait(&iterator);
    
    return (!data.success);
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
