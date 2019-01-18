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

#include "run-deploy.h"
#include <manifest.h>
#include <deploy.h>

static void printProfileArg(const gchar *profile)
{
    if(g_strcmp0(profile, "default") != 0)
         g_printerr(" -p %s", profile);
}

static void printCoordinatorProfilePathArg(const gchar *coordinator_profile_path)
{
    if(coordinator_profile_path != NULL)
        g_printerr(" --coordinator-profile-path %s", coordinator_profile_path);
}

int run_deploy(const gchar *new_manifest, gchar *old_manifest, const gchar *coordinator_profile_path, gchar *profile, const unsigned int max_concurrent_transfers, const int keep, const unsigned int flags)
{
    Manifest *manifest = create_manifest(new_manifest, MANIFEST_ALL_FLAGS, NULL, NULL);

    if(manifest == NULL)
    {
        g_printerr("[coordinator]: Error opening manifest file!\n");
        return 1;
    }
    else
    {
        gchar *old_manifest_file = determine_manifest_to_open(old_manifest, coordinator_profile_path, profile);
        Manifest *previous_manifest = open_previous_manifest(old_manifest_file, MANIFEST_ALL_FLAGS, NULL, NULL);
        int status;

        /* Execute the deployment process */
        status = deploy(old_manifest_file, new_manifest, manifest, previous_manifest, profile, coordinator_profile_path, max_concurrent_transfers, keep, flags);

        switch(status)
        {
            case DEPLOY_OK:
                /* Display warning if state has been moved, but removed from old location */
                if(!(flags & FLAG_DELETE_STATE) && old_manifest_file != NULL)
                {
                    g_printerr(
                    "\nNOTE: Deleting state has been disabled! This means that the state of the\n"
                    "services that have been moved from one machine to another were copied, but not\n"
                    "deleted from their previous locations!\n\n"

                    "To actually remove the old obsolete state, run the following command:\n\n"
                    );

                    g_printerr("$ disnix-delete-state");

                    printProfileArg(profile);
                    printCoordinatorProfilePathArg(coordinator_profile_path);

                    g_printerr(" %s\n\n", old_manifest_file);
                }
                break;
            case DEPLOY_FAIL:
                g_printerr("The deployment failed! Please inspect the output to diagnose any problems!\n");
                break;
            case DEPLOY_STATE_FAIL:
                {
                    g_printerr(
                    "\nThe problem is a data migration issue, which must be manually diagnosed. To\n"
                    "attempt migrating the data again, run:\n\n"
                    );

                    g_printerr("$ disnix-migrate");

                    printProfileArg(profile);
                    printCoordinatorProfilePathArg(coordinator_profile_path);

                    if(flags & FLAG_NO_UPGRADE)
                        g_printerr(" --no-upgrade");

                    if(flags & FLAG_DELETE_STATE)
                        g_printerr(" --delete-state");

                    if(old_manifest_file != NULL)
                        g_printerr(" -o %s", old_manifest_file);

                    g_printerr("%s\n\n", new_manifest);

                    g_printerr("If all problems have been resolved, the deployment must be made final. To\n");
                    g_printerr("accomplish this, run:\n\n");

                    g_printerr("$ disnix-set ");

                    printProfileArg(profile);
                    printCoordinatorProfilePathArg(coordinator_profile_path);

                    g_printerr(" %s\n\n", new_manifest);
                    break;
                }
        }


        /* Cleanup */
        g_free(old_manifest_file);
        delete_manifest(manifest);
        delete_manifest(previous_manifest);

        /* Return the status */
        return status;
    }
}
