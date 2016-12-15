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

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <distributedderivation.h>
#include <derivationmapping.h>
#include <interfaces.h>
#include <client-interface.h>
#include <procreact_pid_iterator.h>
#include <procreact_future_iterator.h>

typedef struct
{
    unsigned int index;
    unsigned int length;
    const GPtrArray *derivation_array;
    const GPtrArray *interface_array;
    int success;
}
DistributionIteratorData;

static int has_next_derivation_item(void *data)
{
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)data;
    return distribution_iterator_data->index < distribution_iterator_data->length;
}

static pid_t next_distribution_process(void *data)
{
    pid_t pid;
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)data;
    
    /* Retrieve derivation item from array */
    DerivationItem *item = g_ptr_array_index(distribution_iterator_data->derivation_array, distribution_iterator_data->index);
    Interface *interface = find_interface(distribution_iterator_data->interface_array, item->target);
    
    /* Execute copy closure process */
    g_print("[target: %s]: Receiving intra-dependency closure of store derivation: %s\n", item->target, item->derivation);
    pid = exec_copy_closure_to(interface->clientInterface, item->target, item->derivation);
    
    distribution_iterator_data->index++;
    return pid;
}

static void complete_distribution_process(void *data, pid_t pid, ProcReact_Status status, int result)
{
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)data;
    
    if(status != PROCREACT_STATUS_OK || !result)
        distribution_iterator_data->success = FALSE;
}

static int distribute_derivations(const GPtrArray *derivation_array, const GPtrArray *interface_array, const unsigned int max_concurrent_transfers)
{
    DistributionIteratorData data = { 0, derivation_array->len, derivation_array, interface_array, TRUE };
    ProcReact_PidIterator iterator = procreact_initialize_pid_iterator(has_next_derivation_item, next_distribution_process, procreact_retrieve_boolean, complete_distribution_process, &data);
    procreact_fork_and_wait_in_parallel_limit(&iterator, max_concurrent_transfers);
    
    return (!data.success);
}

typedef struct
{
    unsigned int index;
    unsigned int length;
    const GPtrArray *derivation_array;
    const GPtrArray *interface_array;
    int success;
    GPtrArray *result_array;
}
RealiseIteratorData;

static ProcReact_Future next_realise_process(void *data)
{
    RealiseIteratorData *realise_iterator_data = (RealiseIteratorData*)data;
    ProcReact_Future future;
    
    DerivationItem *item = g_ptr_array_index(realise_iterator_data->derivation_array, realise_iterator_data->index);
    Interface *interface = find_interface(realise_iterator_data->interface_array, item->target);
    g_print("[target: %s]: Realising derivation: %s\n", item->target, item->derivation);
    future = exec_realise(interface->clientInterface, item->target, item->derivation);
    realise_iterator_data->index++;
    return future;
}

static void complete_realise(void *data, ProcReact_Future *future, ProcReact_Status status)
{
    RealiseIteratorData *realise_iterator_data = (RealiseIteratorData*)data;
    
    if(status == PROCREACT_STATUS_OK && future->result != NULL)
    {
        gchar *result = future->result;
        g_ptr_array_add(realise_iterator_data->result_array, result);
    }
    else
        realise_iterator_data->success = FALSE;
}

static int realise(const GPtrArray *derivation_array, const GPtrArray *interface_array, GPtrArray *result_array)
{
    RealiseIteratorData data = { 0, derivation_array->len, derivation_array, interface_array, TRUE, result_array };
    ProcReact_FutureIterator iterator = procreact_initialize_future_iterator(has_next_derivation_item, next_realise_process, complete_realise, &data);
    
    procreact_fork_in_parallel_buffer_and_wait(&iterator);
    
    procreact_destroy_future_iterator(&iterator);
    return (!data.success);
}

static pid_t next_retrieve_process(void *data)
{
    pid_t pid;
    RealiseIteratorData *realise_iterator_data = (RealiseIteratorData*)data;
    
    gchar *result = g_ptr_array_index(realise_iterator_data->result_array, realise_iterator_data->index);
    DerivationItem *item = g_ptr_array_index(realise_iterator_data->derivation_array, realise_iterator_data->index);
    Interface *interface = find_interface(realise_iterator_data->interface_array, item->target);
    
    g_print("[target: %s]: Sending build result to coordinator: %s\n", item->target, result);
    
    pid = exec_copy_closure_from(interface->clientInterface, item->target, result);
    
    realise_iterator_data->index++;
    return pid;
}

static int retrieve_results(const GPtrArray *derivation_array, const GPtrArray *interface_array, GPtrArray *result_array, const unsigned int max_concurrent_transfers)
{
    RealiseIteratorData data = { 0, derivation_array->len, derivation_array, interface_array, TRUE, result_array };
    ProcReact_PidIterator iterator = procreact_initialize_pid_iterator(has_next_derivation_item, next_retrieve_process, procreact_retrieve_boolean, complete_distribution_process, &data);
    procreact_fork_and_wait_in_parallel_limit(&iterator, max_concurrent_transfers);
    
    return (!data.success);
}

static void delete_result_array(GPtrArray *result_array)
{
    if(result_array != NULL)
    {
        unsigned int i;
        
        for(i = 0; i < result_array->len; i++)
        {
            gchar *result = g_ptr_array_index(result_array, i);
            g_free(result);
        }
    
        g_ptr_array_free(result_array, TRUE);
    }
}

static void cleanup(GPtrArray *result_array, DistributedDerivation *distributed_derivation)
{
    delete_result_array(result_array);
    delete_distributed_derivation(distributed_derivation);
}

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
        GPtrArray *result_array = g_ptr_array_new();
        
        /* Distribute derivations to target machines */
        if((status = distribute_derivations(distributed_derivation->derivation_array, distributed_derivation->interface_array, max_concurrent_transfers)) != 0)
        {
             cleanup(result_array, distributed_derivation);
             return status;
        }
        
        /* Realise derivations on target machines */
        if((status = realise(distributed_derivation->derivation_array, distributed_derivation->interface_array, result_array)) != 0)
        {
            cleanup(result_array, distributed_derivation);
            return status;
        }
        
        /* Retrieve back the build results */
        if((status = retrieve_results(distributed_derivation->derivation_array, distributed_derivation->interface_array, result_array, max_concurrent_transfers)) != 0)
        {
            cleanup(result_array, distributed_derivation);
            return status;
        }
        
        /* Cleanup */
        cleanup(result_array, distributed_derivation);
        
        /* Return the exit status, which is 0 if everything succeeds */
        return 0;
    }
}
