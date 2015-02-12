/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2013  Sander van der Burg
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

#ifndef __DISNIX_TRANSITION_H
#define __DISNIX_TRANSITION_H
#include <glib.h>

/**
 * Performs the transition phase, in which obsolete services are deactivated and
 * new services are activated.
 *
 * @param interface Path to the client interface executable
 * @param new_activation_mappings Array containing the activation mappings of the new configuration
 * @param old_activation_mappings Array containing the activation mappings of the old configuration
 * @param target_array Array containing all the targets of the new configuration
 * @param dry_run Only prints the steps to be executed but does not actually perform them
 * @return 0 if the phase succeeds, else a non-zero exit status
 */
int transition(gchar *interface, GPtrArray *new_activation_mappings, GPtrArray *old_activation_mappings, GPtrArray *target_array, const gboolean dry_run);

#endif
