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

#ifndef __DISNIX_RUN_DIAGNOSE_H
#define __DISNIX_RUN_DIAGNOSE_H
#include <glib.h>

/**
 * Searches for a service that match the service name, container and target and
 * spawns a diagnostic shell for them.
 *
 * @param service_name Name of the service to connect to
 * @param show_mappings Indicates whether to show mapping or to spawn a shell
 * @param manifest_file Path to the manifest file
 * @param coordinator_profile_path Path where the current deployment state is stored for future reference
 * @param profile Identifier of the distributed profile
 * @param container_filter Container to which a service is deployed or NULL to consider all possible containers
 * @param target_filter Target to which a service is deployed or NULL to consider all possible targets
 * @param command Command to execute in the shell or NULL to spawn an interactive shell
 * @return 0 if the operation succeeded, else a non-zero exit status
 */
int diagnose(const char *service_name, const int show_mappings, const char *manifest_file, const gchar *coordinator_profile_path, gchar *profile, const char *container_filter, const char *target_filter, char *command);

#endif
