/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2021  Sander van der Burg
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

#ifndef __DISNIX_DEPLOY_H
#define __DISNIX_DEPLOY_H
#include <glib.h>
#include <manifest.h>

/**
 * @brief Possible outcomes for the deployment operation
 */
typedef enum
{
    DEPLOY_OK,
    DEPLOY_FAIL,
    DEPLOY_STATE_FAIL
}
DeployStatus;

/**
 * Executes all required deployment activites to deploy a configuration
 * described in a manifest file.
 *
 * @param old_manifest_file Path to the old manifest file
 * @param new_manifest_file Path to the new manifest file
 * @param manifest Manifest containing all deployment information of the new configuration
 * @param manifest_old Manifest containing all deployment information of the previous configuration
 * @param profile Name of the distributed profile
 * @param coordinator_profile_path Path where the current deployment configuration must be stored
 * @param max_concurrent_transfers Specifies the maximum amount of concurrent transfers
 * @param tmpdir Directory in which the temp files should be stored
 * @param keep Indicates how many snapshot generations should be kept remotely while executing the depth first operation
 * @param flags Deployment option flags
 * @param pre_hook Pointer to a function that gets executed before a series of critical operations start. This function can be used to catch a SIGINT signal and do a proper rollback. If the pointer is NULL then no function is executed.
 * @param pre_hook Pointer to a function that gets executed after the critical operations are done. This function can be used to restore the handler for the SIGINT to normal. If the pointer is NULL then no function is executed.
 * @return One of the possible outcomes in the DeployStatus enumeration
 */
DeployStatus deploy(gchar *old_manifest_file, const gchar *new_manifest_fike, Manifest *manifest, Manifest *old_manifest, gchar *profile, const gchar *coordinator_profile_path, const unsigned int max_concurrent_transfers, char *tmpdir, const unsigned int keep, const unsigned int flags, void (*pre_hook) (void), void (*post_hook) (void));

#endif
