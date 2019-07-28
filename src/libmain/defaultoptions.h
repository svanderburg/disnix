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

#ifndef __DISNIX_DEFAULTOPTIONS_H
#define __DISNIX_DEFAULTOPTIONS_H

#define DISNIX_DEFAULT_MAX_NUM_OF_CONCURRENT_TRANSFERS 2
#define DISNIX_DEFAULT_KEEP 1
#define DISNIX_DEFAULT_XML FALSE

/**
 * @brief Enumeration of all possible command-line options used in the Disnix toolset
 */
typedef enum
{
    /* Short and long options */

    /* General housekeeping options */
    DISNIX_OPTION_HELP = 'h',
    DISNIX_OPTION_VERSION = 'v',

    /* Connectivity options */
    DISNIX_OPTION_MAX_CONCURRENT_TRANSFERS = 'm',

    /* Configuration options */
    DISNIX_OPTION_PROFILE = 'p',
    DISNIX_OPTION_OLD_MANIFEST = 'o',

    /* State management options */
    DISNIX_OPTION_COMPONENT = 'C',
    DISNIX_OPTION_CONTAINER = 'c',
    DISNIX_OPTION_UNLOCK = 'u',

    /* Documenation options */
    DISNIX_OPTION_FORMAT = 'f',

    /* Garbage collect options */
    DISNIX_OPTION_DELETE_OLD = 'd',

    /* Diagnose options */
    DISNIX_OPTION_SERVICE = 'S',
    DISNIX_OPTION_TARGET = 't',

    /* Long-only options */

    /* Connectivity options */
    DISNIX_OPTION_INTERFACE = 256,
    DISNIX_OPTION_TARGET_PROPERTY = 257,

    /* Deployment options */
    DISNIX_OPTION_NO_UPGRADE = 258,
    DISNIX_OPTION_NO_ROLLBACK = 259,
    DISNIX_OPTION_NO_MIGRATION = 260,
    DISNIX_OPTION_NO_LOCK = 261,
    DISNIX_OPTION_DRY_RUN = 252,

    /* Model options */
    DISNIX_OPTION_XML = 263,

    /* State management options */
    DISNIX_OPTION_KEEP = 264,
    DISNIX_OPTION_DELETE_STATE = 265,
    DISNIX_OPTION_TRANSFER_ONLY = 266,
    DISNIX_OPTION_DEPTH_FIRST = 267,
    DISNIX_OPTION_ALL = 268,

    /* Diagnose options */
    DISNIX_OPTION_SHOW_MAPPINGS = 269,
    DISNIX_OPTION_COMMAND = 270,

    /* Profile management options */
    DISNIX_OPTION_COORDINATOR_PROFILE_PATH = 271,
    DISNIX_OPTION_NO_COORDINATOR_PROFILE = 272,
    DISNIX_OPTION_NO_TARGET_PROFILES = 273,

    /* Visualize options */
    DISNIX_OPTION_NO_CONTAINERS = 274,

    /* Convert options */
    DISNIX_OPTION_INFRASTRUCTURE = 'i'
}
DisnixCommandLineOption;

/**
 * Checks the interface option. If NULL, it will take the value defined in the
 * DISNIX_CLIENT_INTERFACE environment variable or else it will use
 * a default value.
 *
 * @param interface Interface value to check
 * @return The given interface if set, or the default interface through the environment variable
 */
char *check_interface_option(char *interface);

/**
 * Checks the target-property option. If NULL, it will take the value defined in the
 * DISNIX_TARGET_PROPERTY environment variable or else it will use
 * a default value.
 *
 * @param target_property Target property value to check
 * @return @return The given target property if set, or the default target property through the environment variable
 */
char *check_target_property_option(char *target_property);

/**
 * Checks the profile option. If NULL, it will use the default value
 *
 * @param profile Profile to value to check
 * @return The given profile if set, or else the default Disnix profile
 */
char *check_profile_option(char *profile);

/**
 * Checks whether the delete state has been enabled via the DISNIX_DELETE_STATE
 * environment variable.
 *
 * @return TRUE if it has been enabled, else FALSE
 */
int check_global_delete_state(void);

/**
 * Prints the name, version, package and copyright statement for a command
 *
 * @param command Path to the executable
 */
void print_version(const char *command);

#endif
