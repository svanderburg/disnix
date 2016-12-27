/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2016  Sander van der Burg
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
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <activationmapping.h>
#include <targets.h>
#include <client-interface.h>

extern volatile int interrupted;

static void print_activation_step(const int activate, const ActivationMapping *mapping, gchar **arguments, const unsigned int arguments_length)
{
    unsigned int i;
    
    g_print("[target: %s]: ", mapping->target);
    
    if(activate)
        g_print("Activating");
    else
        g_print("Deactivating");
    
    g_print(" service with key: %s and package: %s of type: %s in container: %s, arguments: ", mapping->key, mapping->service, mapping->type, mapping->container);
    
    for(i = 0; i < arguments_length; i++)
        g_print("%s ", arguments[i]);
    
    g_print("\n");
}

static pid_t activate_mapping(ActivationMapping *mapping, Target *target, gchar **arguments, unsigned int arguments_length)
{
    print_activation_step(TRUE, mapping, arguments, arguments_length); /* Print debug message */
    return exec_activate(target->client_interface, mapping->target, mapping->container, mapping->type, arguments, arguments_length, mapping->service);
}

static pid_t dry_run_activate_mapping(ActivationMapping *mapping, Target *target, gchar **arguments, unsigned int arguments_length)
{
    print_activation_step(TRUE, mapping, arguments, arguments_length); /* Print debug message */
    return exec_true(); /* Execute dummy process */
}

static pid_t deactivate_mapping(ActivationMapping *mapping, Target *target, gchar **arguments, unsigned int arguments_length)
{
    print_activation_step(FALSE, mapping, arguments, arguments_length); /* Print debug message */
    return exec_deactivate(target->client_interface, mapping->target, mapping->container, mapping->type, arguments, arguments_length, mapping->service);
}

static pid_t dry_run_deactivate_mapping(ActivationMapping *mapping, Target *target, gchar **arguments, unsigned int arguments_length)
{
    print_activation_step(FALSE, mapping, arguments, arguments_length); /* Print debug message */
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

static int rollback_to_old_mappings(GPtrArray *union_array, GPtrArray *old_activation_mappings, GPtrArray *target_array, const gboolean dry_run)
{
    map_activation_mapping_function map_activation_mapping;
    
    if(dry_run)
        map_activation_mapping = dry_run_activate_mapping;
    else
        map_activation_mapping = activate_mapping;
    
    mark_erroneous_mappings(union_array, ACTIVATIONMAPPING_ACTIVATED); /* Mark erroneous mappings as activated */
    
    if(traverse_activation_mappings(old_activation_mappings, union_array, target_array, traverse_inter_dependency_mappings, map_activation_mapping, complete_activation))
        return 1;
    else
    {
        g_printerr("[coordinator]: Rollback failed!\n\n");
        return ROLLBACK_FAILED;
    }
}

static int deactivate_obsolete_mappings(GPtrArray *deactivation_array, GPtrArray *union_array, GPtrArray *target_array, GPtrArray *old_activation_mappings, const gboolean no_rollback, const gboolean dry_run)
{
    if(deactivation_array == NULL)
        return 0;
    else
    {
        map_activation_mapping_function map_activation_mapping;
        
        if(dry_run)
            map_activation_mapping = dry_run_deactivate_mapping;
        else
            map_activation_mapping = deactivate_mapping;
        
        if(traverse_activation_mappings(deactivation_array, union_array, target_array, traverse_interdependent_mappings, map_activation_mapping, complete_deactivation))
            return 0;
        else
        {
            if(no_rollback)
            {
                g_printerr("[coordinator]: Deactivation failed, but not performing a rollback because it has\n");
                g_printerr("been disabled! Please manually diagnose the problems!\n");
                return 1;
            }
            else
            {
                /* If the deactivation fails, perform a rollback */
                g_printerr("[coordinator]: Deactivation failed! Doing a rollback...\n");
                return rollback_to_old_mappings(union_array, old_activation_mappings, target_array, dry_run);
            }
        }
    }
}

static int rollback_new_mappings(GPtrArray *activation_array, GPtrArray *union_array, GPtrArray *target_array, const gboolean dry_run)
{
    map_activation_mapping_function map_activation_mapping;
    
    if(dry_run)
        map_activation_mapping = dry_run_deactivate_mapping;
    else
        map_activation_mapping = deactivate_mapping;
    
    mark_erroneous_mappings(union_array, ACTIVATIONMAPPING_DEACTIVATED); /* Mark erroneous mappings as deactivated */
    if(traverse_activation_mappings(activation_array, union_array, target_array, traverse_interdependent_mappings, map_activation_mapping, complete_deactivation))
        return 0;
    else
    {
        g_printerr("[coordinator]: Rollback failed!\n\n");
        return ROLLBACK_FAILED;
    }
}

static int activate_new_mappings(GPtrArray *activation_array, GPtrArray *union_array, GPtrArray *target_array, GPtrArray *old_activation_mappings, const gboolean no_rollback, const gboolean dry_run)
{
    map_activation_mapping_function map_activation_mapping;
    
    if(dry_run)
        map_activation_mapping = dry_run_activate_mapping;
    else
        map_activation_mapping = activate_mapping;
    
    if(traverse_activation_mappings(activation_array, union_array, target_array, traverse_inter_dependency_mappings, map_activation_mapping, complete_activation))
        return 0;
    else
    {
        if(no_rollback)
        {
            g_printerr("[coordinator]: Activation failed, but not doing a rollback as it has been\n");
            g_printerr("disabled! Please manually diagnose the errors!\n");
            return 1;
        }
        else
        {
            int status;
            
            /* If the activation fails, perform a rollback */
            g_printerr("[coordinator]: Activation failed! Doing a rollback...\n");
            
            /* Roll back the new mappings */
            status = rollback_new_mappings(activation_array, union_array, target_array, dry_run);
            
            if(status != 0)
                return status; // If the roll back failed, stop and notify the user to take manual action
            
            /* If the new mappings have been rolled backed, roll back to the old mappings */
            if(old_activation_mappings != NULL)
                return rollback_to_old_mappings(union_array, old_activation_mappings, target_array, dry_run);
            else
                return 1;
        }
    }
}

static void cleanup(const GPtrArray *old_activation_mappings, GPtrArray *deactivation_array, GPtrArray *activation_array, GPtrArray *union_array)
{
    if(old_activation_mappings != NULL)
    {
        g_ptr_array_free(deactivation_array, TRUE);
        g_ptr_array_free(activation_array, TRUE);
        g_ptr_array_free(union_array, TRUE);
    }
}

int transition(GPtrArray *new_activation_mappings, GPtrArray *old_activation_mappings, GPtrArray *target_array, const gboolean no_rollback, const gboolean dry_run)
{
    GPtrArray *union_array;
    GPtrArray *deactivation_array;
    GPtrArray *activation_array;
    int status;
    
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

    /* Execute deactivation process */
    g_print("[coordinator]: Executing deactivation of services:\n");
    
    if((status = deactivate_obsolete_mappings(deactivation_array, union_array, target_array, old_activation_mappings, no_rollback, dry_run)) != 0)
    {
        cleanup(old_activation_mappings, deactivation_array, activation_array, union_array);
        return status;
    }
    
    g_print("[coordinator]: Executing activation of services:\n");

    /* Execute activation process (if deactivation process did not fail) */
    if((status = activate_new_mappings(activation_array, union_array, target_array, old_activation_mappings, no_rollback, dry_run)) != 0)
    {
        cleanup(old_activation_mappings, deactivation_array, activation_array, union_array);
        return status;
    }
    
    /* Cleanup */
    cleanup(old_activation_mappings, deactivation_array, activation_array, union_array);
    
    /* Returns the exit status, which is 0 if everything succeeded */
    return 0;
}
