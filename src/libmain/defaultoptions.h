/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2017  Sander van der Burg
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
 * Prints the name, version, package and copyright statement for a command
 *
 * @param command Path to the executable
 */
void print_version(const char *command);

#endif
