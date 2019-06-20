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

#include "transition.h"
#include <activationmapping.h>
#include <targets.h>
#include <client-interface.h>

extern volatile int interrupted;

static void print_activation_step(const gchar *activity, const ActivationMapping *mapping, gchar **arguments, const unsigned int arguments_length)
{
    unsigned int i;
    
    g_print("[target: %s]: %s service with key: %s and package: %s of type: %s in container: %s, arguments: ", mapping->target, activity, mapping->key, mapping->service, mapping->type, mapping->container);
    
    for(i = 0; i < arguments_length; i++)
        g_print("%s ", arguments[i]);
    
    g_print("\n");
}

static pid_t activate_mapping(ActivationMapping *mapping, Target *target, gchar **arguments, const unsigned int arguments_length)
{
    print_activation_step("Activating", mapping, arguments, arguments_length); /* Print debug message */
    return exec_activate((char*)target->client_interface, (char*)mapping->target, (char*)mapping->container, (char*)mapping->type, arguments, arguments_length, (char*)mapping->service);
}

static pid_t dry_run_activate_mapping(ActivationMapping *mapping, Target *target, gchar **arguments, const unsigned int arguments_length)
{
    print_activation_step("Dry-run activating", mapping, arguments, arguments_length); /* Print debug message */
    return exec_true(); /* Execute dummy process */
}

static pid_t deactivate_mapping(ActivationMapping *mapping, Target *target, gchar **arguments, const unsigned int arguments_length)
{
    print_activation_step("Deactivating", mapping, arguments, arguments_length); /* Print debug message */
    return exec_deactivate((char*)target->client_interface, (char*)mapping->target, (char*)mapping->container, (char*)mapping->type, arguments, arguments_length, (char*)mapping->service);
}

static pid_t dry_run_deactivate_mapping(ActivationMapping *mapping, Target *target, gchar **arguments, const unsigned int arguments_length)
{
    print_activation_step("Dry-run deactivating", mapping, arguments, arguments_length); /* Print debug message */
    return exec_true(); /* Execute dummy process */
}

static void complete_activation(ActivationMapping *mapping, ProcReact_Status status, int result)
{
    if(status == PROCREACT_STATUS_OK && result)
        mapping->status = ACTIVATIONMAPPING_ACTIVATED;
    else
    {
        mapping->status = ACTIVATIONMAPPING_ERROR;
        g_printerr("[target: %s]: Activation failed of service: %s\n", mapping->target, mapping->key);
    }
}

static void complete_deactivation(ActivationMapping *mapping, ProcReact_Status status, int result)
{
    if(status == PROCREACT_STATUS_OK && result)
        mapping->status = ACTIVATIONMAPPING_DEACTIVATED;
    else
    {
        mapping->status = ACTIVATIONMAPPING_ERROR;
        g_printerr("[target: %s]: Deactivation failed of service: %s\n", mapping->target, mapping->key);
    }
}

static void mark_erroneous_mappings(GPtrArray *union_array, ActivationMappingStatus status)
{
    unsigned int i;
    
    for(i = 0; i < union_array->len; i++)
    {
        ActivationMapping *mapping = g_ptr_array_index(union_array, i);
        
        if(mapping->status == ACTIVATIONMAPPING_ERROR)
            mapping->status = status;
    }
}

static int rollback_to_old_mappings(GPtrArray *union_array, GPtrArray *old_activation_mappings, GHashTable *targets_table, const unsigned int flags, map_activation_mapping_function activate_mapping_function)
{
    mark_erroneous_mappings(union_array, ACTIVATIONMAPPING_ACTIVATED); /* Mark erroneous mappings as activated */
    return traverse_activation_mappings(old_activation_mappings, union_array, targets_table, traverse_inter_dependency_mappings, activate_mapping_function, complete_activation);
}

static TransitionStatus deactivate_obsolete_mappings(GPtrArray *deactivation_array, GPtrArray *union_array, GHashTable *targets_table, GPtrArray *old_activation_mappings, const unsigned int flags, map_activation_mapping_function activate_mapping_function, map_activation_mapping_function deactivate_mapping_function)
{
    g_print("[coordinator]: Executing deactivation of services:\n");
    
    if(deactivation_array == NULL)
        return TRANSITION_SUCCESS;
    else
    {
        if(traverse_activation_mappings(deactivation_array, union_array, targets_table, traverse_interdependent_mappings, deactivate_mapping_function, complete_deactivation) && !interrupted)
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
                if(rollback_to_old_mappings(union_array, old_activation_mappings, targets_table, flags, activate_mapping_function))
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

static int rollback_new_mappings(GPtrArray *activation_array, GPtrArray *union_array, GHashTable *targets_table, const unsigned int flags, map_activation_mapping_function deactivate_mapping_function)
{
    mark_erroneous_mappings(union_array, ACTIVATIONMAPPING_DEACTIVATED); /* Mark erroneous mappings as deactivated */
    return traverse_activation_mappings(activation_array, union_array, targets_table, traverse_interdependent_mappings, deactivate_mapping_function, complete_deactivation);
}

static TransitionStatus activate_new_mappings(GPtrArray *activation_array, GPtrArray *union_array, GHashTable *targets_table, GPtrArray *old_activation_mappings, const unsigned int flags, map_activation_mapping_function activate_mapping_function, map_activation_mapping_function deactivate_mapping_function)
{
    g_print("[coordinator]: Executing activation of services:\n");
    
    if(traverse_activation_mappings(activation_array, union_array, targets_table, traverse_inter_dependency_mappings, activate_mapping_function, complete_activation) && !interrupted)
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
            if(!rollback_new_mappings(activation_array, union_array, targets_table, flags, deactivate_mapping_function))
            {
                g_printerr("[coordinator]: New mappings rollback failed!\n\n");
                return TRANSITION_NEW_MAPPINGS_ROLLBACK_FAILED; /* If the rollback failed, stop and notify the user to take manual action */
            }
            
            if(old_activation_mappings == NULL)
                return TRANSITION_FAILED;
            else
            {
                /* If the new mappings have been rolled backed, roll back to the old mappings */
                
                if(rollback_to_old_mappings(union_array, old_activation_mappings, targets_table, flags, activate_mapping_function))
                    return TRANSITION_FAILED;
                else
                    return TRANSITION_OBSOLETE_MAPPINGS_ROLLBACK_FAILED;
            }
        }
    }
}

TransitionStatus transition(GPtrArray *new_activation_mappings, GPtrArray *old_activation_mappings, GHashTable *targets_table, const unsigned int flags)
{
    GPtrArray *union_array;
    GPtrArray *deactivation_array;
    GPtrArray *activation_array;
    TransitionStatus status;
    map_activation_mapping_function activate_mapping_function, deactivate_mapping_function;
    
    /* Print configurations */
    
    if(old_activation_mappings == NULL)
    {
        union_array = new_activation_mappings;
        deactivation_array = NULL;
        activation_array = new_activation_mappings;
        
        g_print("[coordinator]: Mapping closures to activate:\n");
        print_activation_array(activation_array);
    }
    else
    {
        GPtrArray *intersection_array = intersect_activation_array(new_activation_mappings, old_activation_mappings);
        
        g_print("[coordinator]: Mapping closures to deactivate:\n");
        deactivation_array = substract_activation_array(old_activation_mappings, intersection_array);
        print_activation_array(deactivation_array);
        
        g_print("[coordinator]: Mapping closures to activate:\n");
        activation_array = substract_activation_array(new_activation_mappings, intersection_array);
        print_activation_array(activation_array);

        union_array = union_activation_array(old_activation_mappings, new_activation_mappings, intersection_array);
        
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
    
    if((status = deactivate_obsolete_mappings(deactivation_array, union_array, targets_table, old_activation_mappings, flags, activate_mapping_function, deactivate_mapping_function)) == TRANSITION_SUCCESS
      && (status = activate_new_mappings(activation_array, union_array, targets_table, old_activation_mappings, flags, activate_mapping_function, deactivate_mapping_function)) == TRANSITION_SUCCESS);
    
    /* Cleanup */
    if(old_activation_mappings != NULL)
    {
        g_ptr_array_free(deactivation_array, TRUE);
        g_ptr_array_free(activation_array, TRUE);
        g_ptr_array_free(union_array, TRUE);
    }

    /* Returns the transition status */
    return status;
}
