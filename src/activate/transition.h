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

#ifndef __DISNIX_TRANSITION_H
#define __DISNIX_TRANSITION_H

#define FLAG_NO_UPGRADE 0x1
#define FLAG_NO_ROLLBACK 0x2
#define FLAG_DRY_RUN 0x4

#include <glib.h>

/**
 * @brief Possible outcomes for the transition process
 */
typedef enum
{
    TRANSITION_SUCCESS = 0,
    TRANSITION_FAILED = 1,
    TRANSITION_OBSOLETE_MAPPINGS_ROLLBACK_FAILED = 2,
    TRANSITION_NEW_MAPPINGS_ROLLBACK_FAILED = 3
}
TransitionStatus;

/**
 * Performs the transition phase, in which obsolete services are deactivated and
 * new services are activated.
 *
 * @param new_activation_mappings Array containing the activation mappings of the new configuration
 * @param old_activation_mappings Array containing the activation mappings of the old configuration
 * @param target_array Array containing all the targets of the new configuration
 * @param flags Option flags
 * @return A status value from the transition status enumeration
 */
TransitionStatus transition(GPtrArray *new_activation_mappings, GPtrArray *old_activation_mappings, GPtrArray *target_array, const unsigned int flags);

#endif
