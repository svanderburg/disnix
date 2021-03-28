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

#include "defaultoptions.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Default interface value */
static char *default_interface = "disnix-ssh-client";

/* Default target property value */
static char *default_target_property = "hostname";

/* Default profile value */
static char *default_profile = "default";

/* Default tmpdir value */
static char *default_tmpdir = "/tmp";

char *check_interface_option(char *interface)
{
    if(interface == NULL)
    {
        char *interface_env = getenv("DISNIX_CLIENT_INTERFACE");

        if(interface_env == NULL)
            interface = default_interface;
        else
            interface = interface_env;
    }

    return interface;
}

char *check_target_property_option(char *target_property)
{
    char *target_property_env = getenv("DISNIX_TARGET_PROPERTY");

    if(target_property_env == NULL)
        target_property = default_target_property;
    else
        target_property = target_property_env;

    return target_property;
}

char *check_profile_option(char *profile)
{
    if(profile == NULL)
    {
        char *profile_env = getenv("DISNIX_PROFILE");

        if(profile_env == NULL)
            profile = default_profile;
        else
            profile = profile_env;
    }

    return profile;
}

disnix_bool check_global_delete_state(void)
{
    return (getenv("DISNIX_DELETE_STATE") != NULL && strcmp(getenv("DISNIX_DELETE_STATE"), "1") == 0);
}

char *check_tmpdir(char *tmpdir)
{
    if(tmpdir == NULL)
    {
        char *tmpdir_env = getenv("TMPDIR");

        if(tmpdir_env == NULL)
            tmpdir = default_tmpdir;
        else
            tmpdir = tmpdir_env;
    }

    return tmpdir;
}

void print_version(const char *command)
{
    printf(
    "%s (" PACKAGE_NAME ") " PACKAGE_VERSION "\n\n"
    "Copyright (C) 2008-2021 Sander van der Burg\n"
    , command);
}
