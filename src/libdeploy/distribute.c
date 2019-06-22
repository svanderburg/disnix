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

#include "distribute.h"
#include <client-interface.h>
#include <distributionmapping-iterator.h>
#include <targets.h>

static pid_t transfer_distribution_item_to(void *data, xmlChar *profile_name, gchar *target_name, Target *target)
{
    char *paths[] = { (char*)profile_name, NULL };
    g_print("[target: %s]: Receiving intra-dependency closure of profile: %s\n", target_name, profile_name);
    return exec_copy_closure_to((char*)target->client_interface, (char*)target_name, paths);
}

static void complete_transfer_distribution_item_to(void *data, xmlChar *profile_name, gchar *target_name, ProcReact_Status status, int result)
{
    if(status != PROCREACT_STATUS_OK || !result)
        g_printerr("[target: %s]: Cannot receive intra-dependency closure of profile: %s\n", target_name, profile_name);
}

int distribute(const Manifest *manifest, const unsigned int max_concurrent_transfers)
{
    /* Iterate over the distribution mappings, limiting concurrency to the desired concurrent transfers and distribute them */
    int success;
    ProcReact_PidIterator iterator = create_distribution_iterator(manifest->distribution_table, manifest->targets_table, transfer_distribution_item_to, complete_transfer_distribution_item_to, NULL);
    procreact_fork_and_wait_in_parallel_limit(&iterator, max_concurrent_transfers);
    success = distribution_iterator_has_succeeded(&iterator);

    /* Delete resources */
    destroy_distribution_iterator(&iterator);

    /* Return status */
    return success;
}
