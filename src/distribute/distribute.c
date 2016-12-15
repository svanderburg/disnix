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

#include "distribute.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <client-interface.h>
#include <manifest.h>
#include <distributionmapping.h>
#include <targets.h>
#include <procreact_pid_iterator.h>

typedef struct
{
    unsigned int index;
    unsigned int length;
    Manifest *manifest;
    int success;
}
DistributionIteratorData;

static int has_next_distribution_item(void *data)
{
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)data;
    return distribution_iterator_data->index < distribution_iterator_data->length;
}

static pid_t next_distribution_process(void *data)
{
    pid_t pid;
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)data;
    
    DistributionItem *item = g_ptr_array_index(distribution_iterator_data->manifest->distribution_array, distribution_iterator_data->index);
    Target *target = find_target(distribution_iterator_data->manifest->target_array, item->target);
    
    g_print("[target: %s]: Receiving intra-dependency closure of profile: %s\n", item->target, item->profile);
    pid = exec_copy_closure_to(target->client_interface, item->target, item->profile);
    distribution_iterator_data->index++;
    return pid;
}

static void complete_distribution_process(void *data, pid_t pid, ProcReact_Status status, int result)
{
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)data;
    
    if(status != PROCREACT_STATUS_OK || !result)
        distribution_iterator_data->success = FALSE;
}

int distribute(const gchar *manifest_file, const unsigned int max_concurrent_transfers)
{
    /* Generate a distribution array from the manifest file */
    Manifest *manifest = create_manifest(manifest_file, NULL, NULL);
    
    if(manifest == NULL)
    {
        g_print("[coordinator]: Error while opening manifest file!\n");
        return 1;
    }
    else
    {
        /* Iterate over the distribution mappings, limiting concurrency to the desired concurrent transfers and distribute them */
        DistributionIteratorData data = { 0, manifest->distribution_array->len, manifest, TRUE };
        ProcReact_PidIterator iterator = procreact_initialize_pid_iterator(has_next_distribution_item, next_distribution_process, procreact_retrieve_boolean, complete_distribution_process, &data);
        procreact_fork_and_wait_in_parallel_limit(&iterator, max_concurrent_transfers);
        
        /* Delete manifest from memory */
        delete_manifest(manifest);
        
        /* Return the exit status, which is 0 if everything succeeds */
        return (!data.success);
    }
}
