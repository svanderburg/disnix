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

#include "build.h"
#include <distributedderivation.h>
#include <derivationmapping.h>
#include <interfaces.h>
#include <client-interface.h>

/* Distribute store derivations infrastructure */

static pid_t copy_derivation_item_to(void *data, DerivationItem *item, Interface *interface)
{
    char *paths[] = { item->derivation, NULL };
    g_print("[target: %s]: Receiving intra-dependency closure of store derivation: %s\n", item->target, item->derivation);
    return exec_copy_closure_to(interface->clientInterface, item->target, paths);
}

static void complete_copy_derivation_item_to(void *data, DerivationItem *item, ProcReact_Status status, int result)
{
    if(status != PROCREACT_STATUS_OK || !result)
        g_printerr("[target: %s]: Cannot receive intra-dependency closure of store derivation: %s\n", item->target, item->derivation);
}

static int distribute_derivations(const GPtrArray *derivation_array, const GPtrArray *interface_array, const unsigned int max_concurrent_transfers)
{
    int success;
    ProcReact_PidIterator iterator = create_derivation_pid_iterator(derivation_array, interface_array, copy_derivation_item_to, complete_copy_derivation_item_to, NULL);
    procreact_fork_and_wait_in_parallel_limit(&iterator, max_concurrent_transfers);
    success = derivation_iterator_has_succeeded(iterator.data);
    
    destroy_derivation_pid_iterator(&iterator);
    return success;
}

/* Realisation infrastructure */

static ProcReact_Future realise_derivation_item(void *data, DerivationItem *item, Interface *interface)
{
    g_print("[target: %s]: Realising derivation: %s\n", item->target, item->derivation);
    return exec_realise(interface->clientInterface, item->target, item->derivation);
}

static void complete_realise_derivation_item(void *data, DerivationItem *item, ProcReact_Future *future, ProcReact_Status status)
{
    if(status == PROCREACT_STATUS_OK && future->result != NULL)
        item->result = future->result;
    else
        g_printerr("[target: %s]: Realising derivation: %s has failed!\n", item->target, item->derivation);
}

static int realise(const GPtrArray *derivation_array, const GPtrArray *interface_array)
{
    int success;
    ProcReact_FutureIterator iterator = create_derivation_future_iterator(derivation_array, interface_array, realise_derivation_item, complete_realise_derivation_item, NULL);
    procreact_fork_in_parallel_buffer_and_wait(&iterator);
    success = derivation_iterator_has_succeeded(iterator.data);
    
    destroy_derivation_future_iterator(&iterator);
    return success;
}

/* Build result retrieval infrastructure */

static pid_t copy_result_from(void *data, DerivationItem *item, Interface *interface)
{
    char *path;
    unsigned int count = 0;
    
    g_print("[target: %s]: Sending build results to coordinator:", item->target);
    
    while((path = item->result[count]) != NULL)
    {
        g_print(" %s", path);
        count++;
    }
    
    g_print("\n");
    
    return exec_copy_closure_from(interface->clientInterface, item->target, item->result);
}

static void complete_copy_result_from(void *data, DerivationItem *item, ProcReact_Status status, int result)
{
    if(status != PROCREACT_STATUS_OK || !result)
        g_print("[target: %s]: Cannot send build result of store derivation to coordinator: %s\n", item->target, item->derivation);
}

static int retrieve_results(const GPtrArray *derivation_array, const GPtrArray *interface_array, const unsigned int max_concurrent_transfers)
{
    int success;
    ProcReact_PidIterator iterator = create_derivation_pid_iterator(derivation_array, interface_array, copy_result_from, complete_copy_result_from, NULL);
    procreact_fork_and_wait_in_parallel_limit(&iterator, max_concurrent_transfers);
    success = derivation_iterator_has_succeeded(iterator.data);
    
    destroy_derivation_pid_iterator(&iterator);
    return success;
}

/* The entire build operation */

int build(const gchar *distributed_derivation_file, const unsigned int max_concurrent_transfers)
{
    DistributedDerivation *distributed_derivation = create_distributed_derivation(distributed_derivation_file);
    
    if(distributed_derivation == NULL)
    {
        g_printerr("[coordinator]: Cannot open distributed derivation file!\n");
        return 1;
    }
    else
    {
        int status;
        
        if(distribute_derivations(distributed_derivation->derivation_array, distributed_derivation->interface_array, max_concurrent_transfers) /* Distribute derivations to target machines */
          && realise(distributed_derivation->derivation_array, distributed_derivation->interface_array) /* Realise derivations on target machines */
          && retrieve_results(distributed_derivation->derivation_array, distributed_derivation->interface_array, max_concurrent_transfers)) /* Retrieve back the build results */
            status = 0;
        else
            status = 1;
            
        /* Cleanup */
        delete_distributed_derivation(distributed_derivation);
        
        /* Return the exit status, which is 0 if everything succeeds */
        return status;
    }
}
