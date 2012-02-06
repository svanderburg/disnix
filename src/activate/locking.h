/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2011  Sander van der Burg
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
 * Unlocks the given profile on each machine defined in the distribution array.
 *
 * @param interface Path to the client interface executable
 * @param distribution_array Array with profiles distributed to the target machines
 * @param profile Identifier of the distributed profile
 * @return 0 if the unlocking phase succeeds, else a non-zero exit status
 */
int unlock(gchar *interface, GArray *distribution_array, gchar *profile);

/**
 * Locks the given profile on each machine defined in the distribution array.
 * In case of a failure the lock is released again.
 *
 * @param interface Path to the client interface executable
 * @param distribution_array Array with profiles distributed to the target machines
 * @param profile Identifier of the distributed profile
 * @return 0 if the unlocking phase succeeds, else a non-zero exit status
 */
int lock(gchar *interface, GArray *distribution_array, gchar *profile);

#endif
