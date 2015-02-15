/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2013  Sander van der Burg
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
#include <manifest.h>
#include <distributionmapping.h>
#include <targets.h>

int distribute(const gchar *manifest_file)
{
    /* Generate a distribution array from the manifest file */
    Manifest *manifest = create_manifest(manifest_file);
    
    if(manifest == NULL)
    {
        g_print("[coordinator]: Error while opening manifest file!\n");
        return 1;
    }
    else
    {
        unsigned int i;
        int exit_status = 0;
        
        /* Iterate over the distribution array and distribute the profiles to the target machines */
        for(i = 0; i < manifest->distribution_array->len; i++)
        {
            DistributionItem *item = g_ptr_array_index(manifest->distribution_array, i);
            Target *target = find_target(manifest->target_array, item->target);
            gchar *interface = find_target_client_interface(target);
            int status;
            
            /* Invoke copy closure operation */
            g_print("[target: %s]: Receiving intra-dependency closure of profile: %s\n", item->target, item->profile);
            status = wait_to_finish(exec_copy_closure_to(interface, item->target, item->profile));
            
            /* On error, change the exit status to indicate an error */
            if(status != 0)
            {
                g_print("[target: %s]: Cannot receive intra-dependency closure\n", item->target);
                exit_status = status;
                break;
            }
        }
        
        /* Delete manifest from memory */
        delete_manifest(manifest);
        
        /* Return the exit status, which is 0 if everything succeeds */
        return exit_status;
    }
}
