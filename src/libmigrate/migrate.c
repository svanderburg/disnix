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

#include "migrate.h"
#include "snapshot.h"
#include "restore.h"
#include "delete-state.h"

ProcReact_bool migrate(const Manifest *manifest, const Manifest *previous_manifest, const unsigned int max_concurrent_transfers, const unsigned int flags, const int keep)
{
    return (snapshot(manifest, previous_manifest, max_concurrent_transfers, flags, keep)
      && restore(manifest, previous_manifest, max_concurrent_transfers, flags, keep)
      && (!(flags & FLAG_DELETE_STATE) || (previous_manifest == NULL) || (flags & FLAG_NO_UPGRADE) || delete_obsolete_state(previous_manifest->snapshot_mapping_array, previous_manifest->services_table, manifest->targets_table)));
}
