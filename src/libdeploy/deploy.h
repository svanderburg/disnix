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

#ifndef __DISNIX_DEPLOY_H
#define __DISNIX_DEPLOY_H
#include <glib.h>
#include <manifest.h>

typedef enum
{
    DEPLOY_OK,
    DEPLOY_FAIL,
    DEPLOY_STATE_FAIL
}
DeployStatus;

DeployStatus deploy(gchar *old_manifest_file, const gchar *new_manifest, Manifest *manifest, Manifest *old_manifest, gchar *profile, const gchar *coordinator_profile_path, const unsigned int max_concurrent_transfers, const unsigned int keep, const unsigned int flags);

#endif
