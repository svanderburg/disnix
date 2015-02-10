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

#include "transition.h"
#include <signal.h>
#include <activationmapping.h>
#include <targets.h>
#include <client-interface.h>

extern volatile int interrupted;

static void print_activation_step(const int activate, const ActivationMapping *mapping, gchar **arguments, const unsigned int arguments_size)
{
    unsigned int i;
    
    g_print("[target: %s]: ", mapping->target);
    
    if(activate)
        g_print("Activating");
    else
        g_print("Deactivating");
    
    g_print(" service with key: %s and package: %s of type: %s, arguments: ", mapping->key, mapping->service, mapping->type);
    
    for(i = 0; i < arguments_size; i++)
        g_print("%s ", arguments[i]);
    
    g_print("\n");
}

static int activate(gchar *interface, GPtrArray *union_array, const ActivationMappingKey *key, const GPtrArray *target_array, const gboolean dry_run)
{
    /* Check for an interruption */
    if(interrupted)
    {
        g_printerr("[coordinator]: The activation process has been interrupted!\n");
        return 1;
    }
    else
    {
        /* Retrieve the mapping from the union array */
        ActivationMapping *actual_mapping = find_activation_mapping(union_array, key);
        
        /* First, activate all inter-dependency mappings */
        if(actual_mapping->depends_on != NULL)
        {
            unsigned int i;
            int status;
            
            for(i = 0; i < actual_mapping->depends_on->len; i++)
            {
                ActivationMappingKey *dependency = g_ptr_array_index(actual_mapping->depends_on, i);
                status = activate(interface, union_array, dependency, target_array, dry_run);
                
                if(status != 0)
                    return status; /* If the activation of an inter-dependency fails, abort */
            }
        }
    
        /* Finally, activate the mapping itself if it is not activated yet */
        if(!actual_mapping->activated)
        {
            unsigned int i;
            int status;
            
            GPtrArray *target = find_target(target_array, actual_mapping->target);
            gchar **arguments = generate_activation_arguments(target); /* Generate an array of key=value pairs from infrastructure properties */
            unsigned int arguments_size = g_strv_length(arguments); /* Determine length of the activation arguments array */
            
            print_activation_step(TRUE, actual_mapping, arguments, arguments_size); /* Print debug message */
            
            if(dry_run)
                status = 0;
            else
                status = wait_to_finish(exec_activate(interface, actual_mapping->target, actual_mapping->type, arguments, arguments_size, actual_mapping->service)); /* Execute the activation operation */
            
            /* Cleanup */
            g_strfreev(arguments);
        
            if(status != 0)
                return status; /* If the activation fails, abort */
            else
                actual_mapping->activated = TRUE; /* Mark activation mapping as activated */
        }
    
        return 0; /* The activation of the closure succeeded */
    }
}

static int deactivate(gchar *interface, GPtrArray *union_array, const ActivationMappingKey *key, const GPtrArray *target_array, const gboolean dry_run)
{
    /* Check for an interruption */
    if(interrupted)
    {
        g_printerr("[coordinator]: The deactivation process has been interrupted!\n");
        return 1;
    }
    else
    {
        /* Retrieve the mapping from the union array */
        ActivationMapping *actual_mapping = find_activation_mapping(union_array, key);
    
        /* Find all interdependent mapping on this mapping */
        GPtrArray *interdependent_mappings = find_interdependent_mappings(union_array, actual_mapping);
    
        /* First deactivate all mappings which have an inter-dependency on this mapping */
    
        unsigned int i;
    
        for(i = 0; i < interdependent_mappings->len; i++)
        {
            ActivationMapping *dependency_mapping = g_ptr_array_index(interdependent_mappings, i);
            int status = deactivate(interface, union_array, (ActivationMappingKey*)dependency_mapping, target_array, dry_run);
        
            if(status != 0)
            {
                g_ptr_array_free(interdependent_mappings, TRUE);
                return status; /* If the deactivation of an inter-dependency fails, abort */
            }
        }
    
        g_ptr_array_free(interdependent_mappings, TRUE);
    
        /* Finally deactivate the mapping itself */
        if(actual_mapping->activated)
        {
            int status;
            GPtrArray *target = find_target(target_array, actual_mapping->target);
            
            if(target == NULL)
            {
                g_print("[target: %s]: Skip deactivation of service with key: %s deploying package: %s since machine is not present!\n", actual_mapping->key, actual_mapping->target, actual_mapping->service);
                status = 0;
            }
            else
            {
                gchar **arguments = generate_activation_arguments(target); /* Generate an array of key=value pairs from infrastructure properties */
                unsigned int arguments_size = g_strv_length(arguments); /* Determine length of the activation arguments array */
                
                print_activation_step(FALSE, actual_mapping, arguments, arguments_size); /* Print debug message */
                
                if(dry_run)
                    status = 0;
                else
                    status = wait_to_finish(exec_deactivate(interface, actual_mapping->target, actual_mapping->type, arguments, arguments_size, actual_mapping->service)); /* Execute the deactivation operation */
                
                /* Cleanup */
                g_free(arguments);
            }

            if(status != 0)
                return status; /* If the deactivation fails, abort */
            else
                actual_mapping->activated = FALSE; /* Mark activation mapping as deactivated */
        }
    
        return 0; /* The deactivation of the closure succeeded */
    }
}

static void rollback_to_old_mappings(GPtrArray *union_array, gchar *interface, GPtrArray *old_activation_mappings, const GPtrArray *target_array, const gboolean dry_run)
{
    unsigned int i;
    
    for(i = 0; i < old_activation_mappings->len; i++)
    {
        ActivationMapping *mapping = g_ptr_array_index(union_array, i);
        
        if(activate(interface, union_array, (ActivationMappingKey*)mapping, target_array, dry_run) != 0)
            g_print("Rollback failed!\n");
    }
}

static int deactivate_obsolete_mappings(GPtrArray *deactivation_array, GPtrArray *union_array, gchar *interface, const GPtrArray *target_array, GPtrArray *old_activation_mappings, const gboolean dry_run)
{
    g_print("[coordinator]: Executing deactivation of services:\n");

    if(deactivation_array == NULL)
        return 0;
    else
    {
        unsigned int i;
        
        /* Deactivate each mapping closure that is not in the new configuration */
        for(i = 0; i < deactivation_array->len; i++)
        {
            ActivationMapping *mapping = g_ptr_array_index(deactivation_array, i);
            int status = deactivate(interface, union_array, (ActivationMappingKey*)mapping, target_array, dry_run);
            
            if(status != 0)
            {
                /* If the deactivation fails, perform a rollback */
                g_print("[coordinator]: Deactivation failed! Doing a rollback...\n");
                rollback_to_old_mappings(union_array, interface, old_activation_mappings, target_array, dry_run);
                return status;
            }
        }
        
        return 0;
    }
}

static void rollback_new_mappings(GPtrArray *activation_array, GPtrArray *union_array, const GPtrArray *target_array, gchar *interface, const gboolean dry_run)
{
    unsigned int i;
    
    for(i = 0; i < activation_array->len; i++)
    {
        ActivationMapping *mapping = g_ptr_array_index(activation_array, i);
        
        if(deactivate(interface, union_array, (ActivationMappingKey*)mapping, target_array, dry_run) != 0)
            g_print("Rollback failed!\n");
    }
}

static int activate_new_mappings(GPtrArray *activation_array, GPtrArray *union_array, gchar *interface, const GPtrArray *target_array, GPtrArray *old_activation_mappings, const gboolean dry_run)
{
    unsigned int i;
    
    g_print("[coordinator]: Executing activation of services:\n");

    /* Activate each mapping closure in the new configuration */
    
    for(i = 0; i < activation_array->len; i++)
    {
        ActivationMapping *mapping = g_ptr_array_index(activation_array, i);
        int status = activate(interface, union_array, (ActivationMappingKey*)mapping, target_array, dry_run);
        
        if(status != 0)
        {
            /* If the activation fails, perform a rollback */
            g_printerr("[coordinator]: Activation failed! Doing a rollback...\n");
            
            rollback_new_mappings(activation_array, union_array, target_array, interface, dry_run);
            
            if(old_activation_mappings != NULL)
                rollback_to_old_mappings(union_array, interface, old_activation_mappings, target_array, dry_run);
        
            return status;
        }
    }
    
    return 0;
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

int transition(gchar *interface, GPtrArray *new_activation_mappings, GPtrArray *old_activation_mappings, const GPtrArray *target_array, const gboolean dry_run)
{
    GPtrArray *union_array;
    GPtrArray *deactivation_array;
    GPtrArray *activation_array;
    int status;
    
    /* Print configurations */
    
    if(old_activation_mappings == NULL)
    {
        unsigned int i;
        
        union_array = new_activation_mappings;
        
        for(i = 0; i < union_array->len; i++)
        {
            ActivationMapping *mapping = g_ptr_array_index(union_array, i);
            mapping->activated = FALSE;
        }
        
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
    if((status = deactivate_obsolete_mappings(deactivation_array, union_array, interface, target_array, old_activation_mappings, dry_run)) != 0)
    {
        cleanup(old_activation_mappings, deactivation_array, activation_array, union_array);
        return status;
    }

    /* Execute activation process (if deactivation process did not fail) */
    if((status = activate_new_mappings(activation_array, union_array, interface, target_array, old_activation_mappings, dry_run)) != 0)
    {
        cleanup(old_activation_mappings, deactivation_array, activation_array, union_array);
        return status;
    }
    
    /* Cleanup */
    cleanup(old_activation_mappings, deactivation_array, activation_array, union_array);
    
    /* Returns the exit status, which is 0 if everything succeeded */
    return 0;
}
