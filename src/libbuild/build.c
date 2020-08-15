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

#include "build.h"
#include <distributedderivation.h>
#include <derivationmapping-iterator.h>
#include <interfacestable.h>
#include <copy-closure.h>
#include <remote-package-management.h>

/* Distribute store derivations infrastructure */

static pid_t copy_derivation_mapping_to(void *data, DerivationMapping *mapping, Interface *interface)
{
    char *tmpdir = (char*)data;
    char *paths[] = { (char*)mapping->derivation, NULL };
    g_print("[target: %s]: Receiving intra-dependency closure of store derivation: %s\n", mapping->interface, mapping->derivation);
    return copy_closure_to((char*)interface->client_interface, (char*)interface->target_address, tmpdir, paths, STDERR_FILENO);
}

static void complete_copy_derivation_mapping_to(void *data, DerivationMapping *mapping, ProcReact_Status status, int result)
{
    if(status != PROCREACT_STATUS_OK || !result)
        g_printerr("[target: %s]: Cannot receive intra-dependency closure of store derivation: %s\n", mapping->interface, mapping->derivation);
}

static int distribute_derivation_mappings(const GPtrArray *derivation_mapping_array, GHashTable *interfaces_table, const unsigned int max_concurrent_transfers, char *tmpdir)
{
    int success;
    ProcReact_PidIterator iterator = create_derivation_mapping_pid_iterator(derivation_mapping_array, interfaces_table, copy_derivation_mapping_to, complete_copy_derivation_mapping_to, tmpdir);

    g_print("[coordinator]: Distributing store derivation files...\n");

    procreact_fork_and_wait_in_parallel_limit(&iterator, max_concurrent_transfers);
    success = derivation_mapping_iterator_has_succeeded(iterator.data);

    destroy_derivation_mapping_pid_iterator(&iterator);
    return success;
}

/* Realisation infrastructure */

static ProcReact_Future realise_derivation_mapping(void *data, DerivationMapping *mapping, Interface *interface)
{
    g_print("[target: %s]: Realising derivation: %s\n", mapping->interface, mapping->derivation);
    return pkgmgmt_remote_realise((char*)interface->client_interface, (char*)interface->target_address, (char*)mapping->derivation);
}

static void complete_realise_derivation_mapping(void *data, DerivationMapping *mapping, ProcReact_Future *future, ProcReact_Status status)
{
    if(status == PROCREACT_STATUS_OK && future->result != NULL)
        mapping->result = future->result;
    else
        g_printerr("[target: %s]: Realising derivation: %s has failed!\n", mapping->interface, mapping->derivation);
}

static int realise(const GPtrArray *derivation_mapping_array, GHashTable *interfaces_table)
{
    int success;
    ProcReact_FutureIterator iterator = create_derivation_mapping_future_iterator(derivation_mapping_array, interfaces_table, realise_derivation_mapping, complete_realise_derivation_mapping, NULL);
    procreact_fork_in_parallel_buffer_and_wait(&iterator);

    g_print("[coordinator]: Realising store derivation files...\n");

    success = derivation_mapping_iterator_has_succeeded(iterator.data);

    destroy_derivation_mapping_future_iterator(&iterator);
    return success;
}

/* Build result retrieval infrastructure */

static pid_t copy_result_from(void *data, DerivationMapping *mapping, Interface *interface)
{
    char *path;
    unsigned int count = 0;

    g_print("[target: %s]: Sending build results to coordinator:", mapping->interface);

    while((path = mapping->result[count]) != NULL)
    {
        g_print(" %s", path);
        count++;
    }

    g_print("\n");

    return copy_closure_from((char*)interface->client_interface, (char*)interface->target_address, mapping->result, STDOUT_FILENO, STDERR_FILENO);
}

static void complete_copy_result_from(void *data, DerivationMapping *mapping, ProcReact_Status status, int result)
{
    if(status != PROCREACT_STATUS_OK || !result)
        g_print("[target: %s]: Cannot send build result of store derivation to coordinator: %s\n", mapping->interface, mapping->derivation);
}

static int retrieve_results(const GPtrArray *derivation_mapping_array, GHashTable *interfaces_table, const unsigned int max_concurrent_transfers)
{
    int success;
    ProcReact_PidIterator iterator = create_derivation_mapping_pid_iterator(derivation_mapping_array, interfaces_table, copy_result_from, complete_copy_result_from, NULL);

    g_print("[coordinator]: Retrieving build results...\n");

    procreact_fork_and_wait_in_parallel_limit(&iterator, max_concurrent_transfers);
    success = derivation_mapping_iterator_has_succeeded(iterator.data);

    destroy_derivation_mapping_pid_iterator(&iterator);
    return success;
}

/* Build orchestration */

int build(DistributedDerivation *distributed_derivation, const unsigned int max_concurrent_transfers, char *tmpdir)
{
    return (distribute_derivation_mappings(distributed_derivation->derivation_mapping_array, distributed_derivation->interfaces_table, max_concurrent_transfers, tmpdir) /* Distribute derivations to target machines */
      && realise(distributed_derivation->derivation_mapping_array, distributed_derivation->interfaces_table) /* Realise derivations on target machines */
      && retrieve_results(distributed_derivation->derivation_mapping_array, distributed_derivation->interfaces_table, max_concurrent_transfers)); /* Retrieve back the build results */
}
