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
#include <client-interface.h>

extern volatile int interrupted;

static void print_activation_step(int activate, gchar **arguments, unsigned int arguments_size, gchar *target_property, ActivationMapping *mapping)
{
    unsigned int i;
    
    g_print("[target: %s]: ", target_property);
    
    if(activate)
        g_print("Activating");
    else
        g_print("Deactivating");
    
    g_print(" service: %s of type: %s, arguments: ", mapping->service, mapping->type);
    
    for(i = 0; i < arguments_size; i++)
        g_print("%s ", arguments[i]);
    
    g_print("\n");
}

static int activate(gchar *interface, GArray *union_array, const ActivationMapping *mapping)
{
    /* Retrieve the mapping from the union array */
    ActivationMapping *actual_mapping = get_activation_mapping(union_array, mapping);

    /* Check for an interruption */
    if(interrupted)
    {
        g_printerr("[coordinator]: The activation process has been interrupted!\n");
        return 1;
    }
    
    /* First, activate all inter-dependency mappings */
    if(actual_mapping->depends_on != NULL)
    {
        unsigned int i;
        int status;
        
        for(i = 0; i < actual_mapping->depends_on->len; i++)
        {
            Dependency *dependency = g_array_index(actual_mapping->depends_on, Dependency*, i);

            ActivationMapping lookup;
            lookup.service = dependency->service;
            lookup.target = dependency->target;

            status = activate(interface, union_array, &lookup);
            
            if(status != 0)
                return status; /* If the activation of an inter-dependency fails, abort */
        }
    }
    
    /* Finally, activate the mapping itself if it is not activated yet */
    if(!actual_mapping->activated)
    {
        unsigned int i;
        int status;
        
        /* Generate an array of key=value pairs from infrastructure properties */
        gchar **arguments = generate_activation_arguments(actual_mapping->target);
        
        /* Determine length of the activation arguments array */
        unsigned int arguments_size = g_strv_length(arguments);
        
        /* Get the target interface property from the mapping */
        gchar *target_property = get_target_property(actual_mapping);
        
        /* Print debug message */
        print_activation_step(TRUE, arguments, arguments_size, target_property, actual_mapping);

        /* Execute the activation operation */
        status = wait_to_finish(exec_activate(interface, target_property, actual_mapping->type, arguments, arguments_size, actual_mapping->service));
        
        /* Cleanup */
        g_strfreev(arguments);
        
        if(status != 0)
            return status; /* If the activation fails, abort */
        else
            actual_mapping->activated = TRUE; /* Mark activation mapping as activated */
    }
    
    return 0; /* The activation of the closure succeeded */
}

static int deactivate(gchar *interface, GArray *union_array, const ActivationMapping *mapping, GArray *target_array)
{
    /* Retrieve the mapping from the union array */
    ActivationMapping *actual_mapping = get_activation_mapping(union_array, mapping);
    
    /* Find all interdependent mapping on this mapping */
    GArray *interdependent_mappings = find_interdependent_mappings(union_array, actual_mapping);
    
    /* First deactivate all mappings which have an inter-dependency on this mapping */
    
    unsigned int i;
    
    for(i = 0; i < interdependent_mappings->len; i++)
    {
        ActivationMapping *dependency_mapping = g_array_index(interdependent_mappings, ActivationMapping*, i);
        int status = deactivate(interface, union_array, dependency_mapping, target_array);
        
        if(status != 0)
        {
            g_array_free(interdependent_mappings, TRUE);
            return status; /* If the deactivation of an inter-dependency fails, abort */
        }
    }
    
    g_array_free(interdependent_mappings, TRUE);
    
    /* Finally deactivate the mapping itself */
    if(actual_mapping->activated)
    {
        int status;
        
        /* Generate an array of key=value pairs from infrastructure properties */
        gchar **arguments = generate_activation_arguments(actual_mapping->target);
        
        /* Determine length of the activation arguments array */
        unsigned int arguments_size = g_strv_length(arguments);
        
        /* Get the target interface property from the mapping */
        gchar *target_property = get_target_property(actual_mapping);
        
        /* Print debug message */
        print_activation_step(FALSE, arguments, arguments_size, target_property, actual_mapping);
        
        if(target_index(target_array, target_property) == -1) /* Only deactivate services on machines that are available */
        {
            g_print("[target: %s]: Skip deactivation of service: %s since machine is not present!\n", target_property, actual_mapping->service);
            status = 0;
        }
        else
            status = wait_to_finish(exec_deactivate(interface, target_property, actual_mapping->type, arguments, arguments_size, actual_mapping->service)); /* Execute the deactivation operation */
        
        /* Cleanup */
        g_free(arguments);

        if(status != 0)
            return status; /* If the deactivation fails, abort */
        else
            actual_mapping->activated = FALSE; /* Mark activation mapping as deactivated */
    }
    
    return 0; /* The deactivation of the closure succeeded */
}

static void rollback_to_old_mappings(GArray *union_array, gchar *interface, GArray *old_activation_mappings)
{
    unsigned int i;
    
    for(i = 0; i < old_activation_mappings->len; i++)
    {
        ActivationMapping *mapping = g_array_index(union_array, ActivationMapping*, i);
        
        if(activate(interface, union_array, mapping) != 0)
            g_print("Rollback failed!\n");
    }
}

static int deactivate_obsolete_mappings(GArray *deactivation_array, GArray *union_array, gchar *interface, GArray *target_array, GArray *old_activation_mappings)
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
            ActivationMapping *mapping = g_array_index(deactivation_array, ActivationMapping*, i);
            int status = deactivate(interface, union_array, mapping, target_array);
            
            if(status != 0)
            {
                /* If the deactivation fails, perform a rollback */
                g_print("[coordinator]: Deactivation failed! Doing a rollback...\n");
                rollback_to_old_mappings(union_array, interface, old_activation_mappings);
                return status;
            }
        }
        
        return 0;
    }
}

static void rollback_new_mappings(GArray *activation_array, GArray *union_array, GArray *target_array, gchar *interface)
{
    unsigned int i;
    
    for(i = 0; i < activation_array->len; i++)
    {
        ActivationMapping *mapping = g_array_index(activation_array, ActivationMapping*, i);
        
        if(deactivate(interface, union_array, mapping, target_array) != 0)
            g_print("Rollback failed!\n");
    }
}

static int activate_new_mappings(GArray *activation_array, GArray *union_array, gchar *interface, GArray *target_array, GArray *old_activation_mappings)
{
    unsigned int i;
    
    g_print("[coordinator]: Executing activation of services:\n");

    /* Activate each mapping closure in the new configuration */
    
    for(i = 0; i < activation_array->len; i++)
    {
        ActivationMapping *mapping = g_array_index(activation_array, ActivationMapping*, i);
        int status = activate(interface, union_array, mapping);
        
        if(status != 0)
        {
            /* If the activation fails, perform a rollback */
            g_printerr("[coordinator]: Activation failed! Doing a rollback...\n");
            
            rollback_new_mappings(activation_array, union_array, target_array, interface);
            
            if(old_activation_mappings != NULL)
                rollback_to_old_mappings(union_array, interface, old_activation_mappings);
        
            return status;
        }
    }
    
    return 0;
}

static void cleanup(GArray *old_activation_mappings, GArray *deactivation_array, GArray *activation_array, GArray *union_array)
{
    if(old_activation_mappings != NULL)
    {
        g_array_free(deactivation_array, TRUE);
        g_array_free(activation_array, TRUE);
        g_array_free(union_array, TRUE);
    }
}

int transition(gchar *interface, GArray *new_activation_mappings, GArray *old_activation_mappings, GArray *target_array)
{
    GArray *union_array;
    GArray *deactivation_array;
    GArray *activation_array;
    int status;
    
    /* Print configurations */
    
    g_print("New configuration mappings:\n");
    print_activation_array(new_activation_mappings);
    
    if(old_activation_mappings == NULL)
    {
        unsigned int i;
        
        union_array = new_activation_mappings;
        
        for(i = 0; i < union_array->len; i++)
        {
            ActivationMapping *mapping = g_array_index(union_array, ActivationMapping*, i);
            mapping->activated = FALSE;
        }
        
        deactivation_array = NULL;
        activation_array = new_activation_mappings;
    }
    else
    {
        GArray *intersection_array;
        
        g_print("Old configuration mappings:\n");
        print_activation_array(old_activation_mappings);

        g_print("Intersection between old and new mappings:\n");
        intersection_array = intersect_activation_array(new_activation_mappings, old_activation_mappings);
        print_activation_array(intersection_array);
        
        g_print("Mapping closures to deactivate:\n");
        deactivation_array = substract_activation_array(old_activation_mappings, intersection_array);
        print_activation_array(deactivation_array);
        
        g_print("Mapping closures to activate:\n");
        activation_array = substract_activation_array(new_activation_mappings, intersection_array);
        print_activation_array(activation_array);

        g_print("Union of old and new configuration mappings:\n");
        union_array = union_activation_array(old_activation_mappings, new_activation_mappings, intersection_array);
        print_activation_array(union_array);
        
        /* Remove obsolete intersection array */
        g_array_free(intersection_array, TRUE);
    }

    /* Execute deactivation process */
    if((status = deactivate_obsolete_mappings(deactivation_array, union_array, interface, target_array, old_activation_mappings)) != 0)
    {
        cleanup(old_activation_mappings, deactivation_array, activation_array, union_array);
        return status;
    }

    /* Execute activation process (if deactivation process did not fail) */
    if((status = activate_new_mappings(activation_array, union_array, interface, target_array, old_activation_mappings)) != 0)
    {
        cleanup(old_activation_mappings, deactivation_array, activation_array, union_array);
        return status;
    }
    
    /* Cleanup */
    cleanup(old_activation_mappings, deactivation_array, activation_array, union_array);
    
    /* Returns the exit status, which is 0 if everything succeeded */
    return 0;
}
