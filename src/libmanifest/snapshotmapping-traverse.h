/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2020  Sander van der Burg
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

#ifndef __DISNIX_SNAPSHOTMAPPING_TRAVERSE_H
#define __DISNIX_SNAPSHOTMAPPING_TRAVERSE_H
#include <glib.h>
#include <libxml/parser.h>
#include <procreact_pid.h>
#include <targetstable.h>
#include "snapshotmappingarray.h"
#include "manifestservice.h"

/**
 * Function that spawns a process for a snapshot mapping.
 *
 * @param mapping A snapshot mapping from a snapshots array
 * @param service Service that is mapped to the machine
 * @param target Target machine to which the snapshot is mapped
 * @param type Module that executes state deployment activities
 * @param arguments Arguments passed to the client interface
 * @param arguments_length Length of the arguments array
 * @return PID of the spawned process
 */
typedef pid_t (*map_snapshot_item_function) (SnapshotMapping *mapping, ManifestService *service, Target *target, xmlChar *type, xmlChar **arguments, unsigned int arguments_length);

/**
 * Function that gets executed when a mapping function completes.
 *
 * @param mapping A snapshot mapping from a snapshots array
 * @param service Service that is mapped to the machine
 * @param target Target machine to which the snapshot is mapped
 * @param status Indicates whether the process terminated abnormally or not
 * @param result TRUE if the mapping operation succeeded, else FALSE
 */
typedef void (*complete_snapshot_item_mapping_function) (SnapshotMapping *mapping, ManifestService *service, Target *target, ProcReact_Status status, int result);

/**
 * Maps over each snapshot mapping, asynchronously executes a function for each
 * item and ensures that for each machine only the allowed number of processes
 * are executed concurrently.
 *
 * @param snapshot_mapping_array Snapshot mapping array
 * @param services_table Hash table of services
 * @param targets_table Hash table of targets
 * @param map_snapshot_item Function that gets executed for each snapshot item
 * @param complete_snapshot_item_mapping Function that gets executed when a mapping function completes
 * @return TRUE if all mappings were successfully executed, else FALSE
 */
int map_snapshot_items(const GPtrArray *snapshot_mapping_array, GHashTable *services_table, GHashTable *targets_table, map_snapshot_item_function map_snapshot_item, complete_snapshot_item_mapping_function complete_snapshot_item_mapping);

#endif
