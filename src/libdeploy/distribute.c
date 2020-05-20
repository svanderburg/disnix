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

#include "distribute.h"
#include <remote-package-management.h>
#include <profilemapping-iterator.h>
#include <targetstable.h>
#include <copy-closure.h>

static pid_t transfer_profile_mapping_to(void *data, gchar *target_name, xmlChar *profile_path, Target *target)
{
    char *tmpdir = (char*)data;
    char *paths[] = { (char*)profile_path, NULL };
    gchar *target_key = find_target_key(target);
    g_print("[target: %s]: Receiving intra-dependency closure of profile: %s\n", target_name, profile_path);
    return copy_closure_to((char*)target->client_interface, target_key, tmpdir, paths);
}

static void complete_transfer_profile_mapping_to(void *data, gchar *target_name, xmlChar *profile_path, Target *target, ProcReact_Status status, int result)
{
    if(status != PROCREACT_STATUS_OK || !result)
        g_printerr("[target: %s]: Cannot receive intra-dependency closure of profile: %s\n", target_name, profile_path);
}

int distribute(const Manifest *manifest, const unsigned int max_concurrent_transfers, char *tmpdir)
{
    /* Iterate over the profile mappings, limiting concurrency to the desired concurrent transfers and distribute them */
    int success;
    ProcReact_PidIterator iterator = create_profile_mapping_iterator(manifest->profile_mapping_table, manifest->targets_table, transfer_profile_mapping_to, complete_transfer_profile_mapping_to, tmpdir);
    procreact_fork_and_wait_in_parallel_limit(&iterator, max_concurrent_transfers);
    success = profile_mapping_iterator_has_succeeded(&iterator);

    /* Delete resources */
    destroy_profile_mapping_iterator(&iterator);

    /* Return status */
    return success;
}
