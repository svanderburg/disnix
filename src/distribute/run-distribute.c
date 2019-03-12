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

#include "run-distribute.h"
#include <client-interface.h>
#include <manifest.h>
#include "distribute.h"

int run_distribute(const gchar *manifest_file, const unsigned int max_concurrent_transfers)
{
    /* Generate a distribution array from the manifest file */
    Manifest *manifest = create_manifest(manifest_file, MANIFEST_DISTRIBUTION_FLAG | MANIFEST_TARGETS_FLAG, NULL, NULL);

    if(manifest == NULL)
    {
        g_print("[coordinator]: Error while opening manifest file!\n");
        return 1;
    }
    else
    {
        /* Iterate over the distribution mappings, limiting concurrency to the desired concurrent transfers and distribute them */
        int exit_status = !distribute(manifest, max_concurrent_transfers);

        /* Delete resources */
        delete_manifest(manifest);

        /* Return the exit status, which is 0 if everything succeeds */
        return exit_status;
    }
}
