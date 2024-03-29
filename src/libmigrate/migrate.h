/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2022  Sander van der Burg
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

#ifndef __DISNIX_MIGRATE_H
#define __DISNIX_MIGRATE_H
#include <glib.h>
#include <procreact_types.h>
#include <manifest.h>
#include "datamigrationflags.h"

/**
 * Migrates the state of all stateful services from the previous locations to
 * the desired locations.
 *
 * @param manifest Manifest containing all deployment information
 * @param old_snapshots_array Array of stateful components belonging to the previous configurations
 * @param max_concurrent_transfers Specifies the maximum amount of concurrent transfers
 * @param flags Data migration option flags
 * @param keep Indicates how many snapshot generations should be kept remotely while executing the depth first operation
 * @return TRUE if the migration completed successfully, else FALSE
 */
ProcReact_bool migrate(const Manifest *manifest, const Manifest *previous_manifest, const unsigned int max_concurrent_transfers, const unsigned int flags, const int keep);

#endif
