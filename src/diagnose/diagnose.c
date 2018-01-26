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

#include "diagnose.h"
#include "manifest.h"
#include "activationmapping.h"
#include "targets.h"
#include "client-interface.h"
#include "procreact_pid.h"

int diagnose(char *service_name, const char *manifest_file, const gchar *coordinator_profile_path, gchar *profile, char *container_filter, char *target_filter, char *command)
{
    int exit_status;
    Manifest *manifest = open_provided_or_previous_manifest_file(manifest_file, coordinator_profile_path, profile, MANIFEST_ACTIVATION_FLAG, NULL, NULL);

    if(manifest == NULL)
    {
        g_printerr("Cannot open manifest file!\n");
        exit_status = 1;
    }
    else
    {
        unsigned int i;
        GPtrArray *candidate_mappings_array = g_ptr_array_new();

        for(i = 0; i < manifest->activation_array->len; i++)
        {
            ActivationMapping *mapping = (ActivationMapping*)g_ptr_array_index(manifest->activation_array, i);

            if(g_strcmp0(service_name, mapping->name) == 0 &&
              (container_filter == NULL || g_strcmp0(container_filter, mapping->container) == 0) &&
              (target_filter == NULL || g_strcmp0(target_filter, mapping->target) == 0))
                g_ptr_array_add(candidate_mappings_array, mapping);
        }

        if(candidate_mappings_array->len == 0)
        {
            g_printerr("No mapping found that matches the provided selection criteria!\n");
            exit_status = 1;
        }
        else if(candidate_mappings_array->len == 1)
        {
            ActivationMapping *mapping = g_ptr_array_index(candidate_mappings_array, 0);
            Target *target = find_target(manifest->target_array, mapping->target);
            gchar **arguments = generate_activation_arguments(target, mapping->container);
            ProcReact_Status status;

            g_printerr("[%s]: Connecting to service: %s deployed to container: %s\n", mapping->target, mapping->service, mapping->container);

            exit_status = procreact_wait_for_exit_status(exec_dysnomia_shell(target->client_interface, mapping->target, mapping->container, mapping->type, arguments, g_strv_length(arguments), mapping->service, command), &status);
            g_strfreev(arguments);
        }
        else
        {
            g_printerr("Multiple mappings found! Please specify a --target and, optionally, a --container parameter!\n\n");
            g_printerr("This service has been mapped to:\n\n");

            for(i = 0; i < candidate_mappings_array->len; i++)
            {
                ActivationMapping *mapping = (ActivationMapping*)g_ptr_array_index(candidate_mappings_array, i);
                g_printerr("container: %s, target: %s\n", mapping->container, mapping->target);
            }

            exit_status = 1;
        }

        g_ptr_array_free(candidate_mappings_array, TRUE);
    }

    delete_manifest(manifest);

    return exit_status;
}
