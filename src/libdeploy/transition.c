/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2022  Sander van der Burg
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

#include "transition.h"
#include <servicemapping-traverse.h>
#include <manifestservicestable.h>
#include <targetstable.h>
#include <remote-state-management.h>

extern volatile int interrupted;

static void print_activation_step(const gchar *activity, const ServiceMapping *mapping, const ManifestService *service, xmlChar *type, xmlChar **arguments, const unsigned int arguments_length)
{
    unsigned int i;

    g_print("[target: %s]: %s service with key: %s and package: %s with module: %s in container: %s, arguments: ", mapping->target, activity, mapping->service, service->pkg, type, mapping->container);

    for(i = 0; i < arguments_length; i++)
        g_print("%s ", arguments[i]);

    g_print("\n");
}

static pid_t activate_mapping(ServiceMapping *mapping, ManifestService *service, Target *target, xmlChar *type, xmlChar **arguments, const unsigned int arguments_length)
{
    gchar *target_key = find_target_key(target);
    print_activation_step("Activating", mapping, service, type, arguments, arguments_length); /* Print debug message */
    return statemgmt_remote_activate((char*)target->client_interface, target_key, (char*)mapping->container, (char*)type, (char**)arguments, arguments_length, (char*)service->pkg);
}

static pid_t dry_run_activate_mapping(ServiceMapping *mapping, ManifestService *service, Target *target, xmlChar *type, xmlChar **arguments, const unsigned int arguments_length)
{
    print_activation_step("Dry-run activating", mapping, service, type, arguments, arguments_length); /* Print debug message */
    return statemgmt_dummy_command(); /* Execute dummy process */
}

static pid_t deactivate_mapping(ServiceMapping *mapping, ManifestService *service, Target *target, xmlChar *type, xmlChar **arguments, const unsigned int arguments_length)
{
    gchar *target_key = find_target_key(target);
    print_activation_step("Deactivating", mapping, service, type, arguments, arguments_length); /* Print debug message */
    return statemgmt_remote_deactivate((char*)target->client_interface, target_key, (char*)mapping->container, (char*)type, (char**)arguments, arguments_length, (char*)service->pkg);
}

static pid_t dry_run_deactivate_mapping(ServiceMapping *mapping, ManifestService *service, Target *target, xmlChar *type, xmlChar **arguments, const unsigned int arguments_length)
{
    print_activation_step("Dry-run deactivating", mapping, service, type, arguments, arguments_length); /* Print debug message */
    return statemgmt_dummy_command(); /* Execute dummy process */
}

static void complete_activation(ServiceMapping *mapping, ManifestService *service, Target *target, ProcReact_Status status, int result)
{
    if(status == PROCREACT_STATUS_OK && result)
        mapping->status = SERVICE_MAPPING_ACTIVATED;
    else
    {
        mapping->status = SERVICE_MAPPING_ERROR;
        g_printerr("[target: %s]: Activation failed of service: %s\n", mapping->target, mapping->service);
    }
}

static void complete_deactivation(ServiceMapping *mapping, ManifestService *service, Target *target, ProcReact_Status status, int result)
{
    if(status == PROCREACT_STATUS_OK && result)
        mapping->status = SERVICE_MAPPING_DEACTIVATED;
    else
    {
        mapping->status = SERVICE_MAPPING_ERROR;
        g_printerr("[target: %s]: Deactivation failed of service: %s\n", mapping->target, mapping->service);
    }
}

static void mark_erroneous_mappings(GPtrArray *unified_service_mapping_array, ServiceMappingStatus status)
{
    unsigned int i;

    for(i = 0; i < unified_service_mapping_array->len; i++)
    {
        ServiceMapping *mapping = g_ptr_array_index(unified_service_mapping_array, i);

        if(mapping->status == SERVICE_MAPPING_ERROR)
            mapping->status = status;
    }
}

static int rollback_to_old_mappings(GPtrArray *unified_service_mapping_array, GHashTable *unified_services_table, GPtrArray *old_activation_mappings, GHashTable *targets_table, const unsigned int flags, service_mapping_function activate_mapping_function)
{
    mark_erroneous_mappings(unified_service_mapping_array, SERVICE_MAPPING_ACTIVATED); /* Mark erroneous mappings as activated */
    return traverse_service_mappings(old_activation_mappings, unified_service_mapping_array, unified_services_table, targets_table, traverse_inter_dependency_mappings, activate_mapping_function, complete_activation);
}

static TransitionStatus deactivate_obsolete_mappings(GPtrArray *deactivation_array, GPtrArray *unified_service_mapping_array, GHashTable *unified_services_table, GHashTable *targets_table, GPtrArray *old_activation_mappings, const unsigned int flags, service_mapping_function activate_mapping_function, service_mapping_function deactivate_mapping_function)
{
    g_print("[coordinator]: Executing deactivation of services:\n");

    if(deactivation_array == NULL)
        return TRANSITION_SUCCESS;
    else
    {
        if(traverse_service_mappings(deactivation_array, unified_service_mapping_array, unified_services_table, targets_table, traverse_interdependent_mappings, deactivate_mapping_function, complete_deactivation) && !interrupted)
            return TRANSITION_SUCCESS;
        else
        {
            if(interrupted)
                g_printerr("[coordinator]: The deactivation phase has been interrupted, reactivating services again...\n");

            if(flags & FLAG_NO_ROLLBACK)
            {
                g_printerr("[coordinator]: Deactivation failed, but not performing a rollback because it has\n");
                g_printerr("been disabled! Please manually diagnose the problems!\n");
                return TRANSITION_FAILED;
            }
            else
            {
                /* If the deactivation fails, perform a rollback */
                g_printerr("[coordinator]: Deactivation failed! Doing a rollback...\n");
                if(rollback_to_old_mappings(unified_service_mapping_array, unified_services_table, old_activation_mappings, targets_table, flags, activate_mapping_function))
                    return TRANSITION_FAILED;
                else
                {
                    g_printerr("[coordinator]: Obsolete mappings rollback failed!\n\n");
                    return TRANSITION_OBSOLETE_MAPPINGS_ROLLBACK_FAILED;
                }
            }
        }
    }
}

static int rollback_new_mappings(GPtrArray *activation_array, GPtrArray *unified_service_mapping_array, GHashTable *unified_services_table, GHashTable *targets_table, const unsigned int flags, service_mapping_function deactivate_mapping_function)
{
    mark_erroneous_mappings(unified_service_mapping_array, SERVICE_MAPPING_DEACTIVATED); /* Mark erroneous mappings as deactivated */
    return traverse_service_mappings(activation_array, unified_service_mapping_array, unified_services_table, targets_table, traverse_interdependent_mappings, deactivate_mapping_function, complete_deactivation);
}

static TransitionStatus activate_new_mappings(GPtrArray *activation_array, GPtrArray *unified_service_mapping_array, GHashTable *unified_services_table, GHashTable *targets_table, GPtrArray *old_activation_mappings, const unsigned int flags, service_mapping_function activate_mapping_function, service_mapping_function deactivate_mapping_function)
{
    g_print("[coordinator]: Executing activation of services:\n");

    if(traverse_service_mappings(activation_array, unified_service_mapping_array, unified_services_table, targets_table, traverse_inter_dependency_mappings, activate_mapping_function, complete_activation) && !interrupted)
        return TRANSITION_SUCCESS;
    else
    {
        if(interrupted)
            g_printerr("[coordinator]: The activation has been interrupted, reverting back to the old state...\n");

        if(flags & FLAG_NO_ROLLBACK)
        {
            g_printerr("[coordinator]: Activation failed, but not doing a rollback as it has been\n");
            g_printerr("disabled! Please manually diagnose the errors!\n");
            return TRANSITION_FAILED;
        }
        else
        {
            /* If the activation fails, perform a rollback */
            g_printerr("[coordinator]: Activation failed! Doing a rollback...\n");

            /* Roll back the new mappings */
            if(!rollback_new_mappings(activation_array, unified_service_mapping_array, unified_services_table, targets_table, flags, deactivate_mapping_function))
            {
                g_printerr("[coordinator]: New mappings rollback failed!\n\n");
                return TRANSITION_NEW_MAPPINGS_ROLLBACK_FAILED; /* If the rollback failed, stop and notify the user to take manual action */
            }

            if(old_activation_mappings == NULL)
                return TRANSITION_FAILED;
            else
            {
                /* If the new mappings have been rolled backed, roll back to the old mappings */

                if(rollback_to_old_mappings(unified_service_mapping_array, unified_services_table, old_activation_mappings, targets_table, flags, activate_mapping_function))
                    return TRANSITION_FAILED;
                else
                    return TRANSITION_OBSOLETE_MAPPINGS_ROLLBACK_FAILED;
            }
        }
    }
}

TransitionStatus transition(Manifest *manifest, Manifest *previous_manifest, const unsigned int flags)
{
    GPtrArray *unified_service_mapping_array;
    GPtrArray *deactivation_array;
    GPtrArray *activation_array;
    GHashTable *unified_services_table;
    GPtrArray *previous_service_mapping_array;
    TransitionStatus status;
    service_mapping_function activate_mapping_function, deactivate_mapping_function;

    /* Print configurations */

    if(previous_manifest == NULL)
    {
        unified_service_mapping_array = manifest->service_mapping_array;
        deactivation_array = NULL;
        activation_array = manifest->service_mapping_array;
        unified_services_table = manifest->services_table;
        previous_service_mapping_array = NULL;
    }
    else
    {
        GPtrArray *intersection_array = intersect_service_mapping_array(manifest->service_mapping_array, previous_manifest->service_mapping_array);
        previous_service_mapping_array = previous_manifest->service_mapping_array;

        deactivation_array = substract_service_mapping_array(previous_manifest->service_mapping_array, intersection_array);
        activation_array = substract_service_mapping_array(manifest->service_mapping_array, intersection_array);

        unified_service_mapping_array = unify_service_mapping_array(previous_manifest->service_mapping_array, manifest->service_mapping_array, intersection_array);
        unified_services_table = generate_union_services_table(manifest->services_table, previous_manifest->services_table);

        /* Remove obsolete intersection array */
        g_ptr_array_free(intersection_array, TRUE);
    }

    /* Determine the activation and deactivation mapping functions */

    if(flags & FLAG_DRY_RUN)
    {
        activate_mapping_function = dry_run_activate_mapping;
        deactivate_mapping_function = dry_run_deactivate_mapping;
    }
    else
    {
        activate_mapping_function = activate_mapping;
        deactivate_mapping_function = deactivate_mapping;
    }

    /* Execute transition steps */
    if((status = deactivate_obsolete_mappings(deactivation_array, unified_service_mapping_array, unified_services_table, manifest->targets_table, previous_service_mapping_array, flags, activate_mapping_function, deactivate_mapping_function)) == TRANSITION_SUCCESS
      && (status = activate_new_mappings(activation_array, unified_service_mapping_array, unified_services_table, manifest->targets_table, previous_service_mapping_array, flags, activate_mapping_function, deactivate_mapping_function)) == TRANSITION_SUCCESS)
        ;

    /* Cleanup */
    if(previous_manifest != NULL)
    {
        g_ptr_array_free(deactivation_array, TRUE);
        g_ptr_array_free(activation_array, TRUE);
        g_ptr_array_free(unified_service_mapping_array, TRUE);
        g_hash_table_destroy(unified_services_table);
    }

    /* Returns the transition status */
    return status;
}
