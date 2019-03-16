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

#include "run-build.h"
#include <build.h>
#include <distributedderivation.h>
#include <derivationmapping.h>
#include <interfaces.h>
#include <client-interface.h>

int run_build(const gchar *distributed_derivation_file, const unsigned int max_concurrent_transfers)
{
    DistributedDerivation *distributed_derivation = create_distributed_derivation(distributed_derivation_file);

    if(distributed_derivation == NULL)
    {
        g_printerr("[coordinator]: Cannot open distributed derivation file!\n");
        return 1;
    }
    else
    {
        int exit_status;

        if(check_distributed_derivation(distributed_derivation))
            exit_status = !build(distributed_derivation, max_concurrent_transfers); /* Execute remote builds */
        else
            exit_status = 1;

        /* Cleanup */
        delete_distributed_derivation(distributed_derivation);

        /* Return the exit status, which is 0 if everything succeeds */
        return exit_status;
    }
}
