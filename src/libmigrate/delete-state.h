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

#ifndef __DISNIX_DELETE_STATE_H
#define __DISNIX_DELETE_STATE_H
#include <glib.h>
#include <procreact_types.h>
#include "manifest.h"

/**
 * Actually deletes all the state that has been marked as obsolete.
 *
 * @param snapshots_array Array of stateful components belonging to the current configurations
 * @param targets_table Hash table of targets belonging to the current configuration
 * @return TRUE if deleting the state completed successfully, else FALSE
 */
ProcReact_bool delete_obsolete_state(GPtrArray *snapshot_mapping_array, GHashTable *services_table, GHashTable *targets_table);

#endif
