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

#ifndef __DISNIX_LOCKING_H
#define __DISNIX_LOCKING_H
#include <glib.h>

/**
 * Unlocks the target machine and all services on all target machines in the
 * network.
 *
 * @param distribution_array Array of distribution items
 * @param target_array Array of targets belonging to the current configuration
 * @param profile Identifier of the distributed profile
 * @return TRUE if all the target machines have been successfully unlocked, else FALSE
 */
int unlock(const GPtrArray *distribution_array, const GPtrArray *target_array, gchar *profile);

/**
 * Locks the target machine and all services on all target machines in the
 * network.
 *
 * @param distribution_array Array of distribution items
 * @param target_array Array of targets belonging to the current configuration
 * @param profile Identifier of the distributed profile
 * @return TRUE if all the target machines have been successfully locked, else FALSE
 */
int lock(const GPtrArray *distribution_array, const GPtrArray *target_array, gchar *profile);

#endif
