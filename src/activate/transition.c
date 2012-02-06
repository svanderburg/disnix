/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2011  Sander van der Burg
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
#include <client-interface.h>

static int activate(gchar *interface, GArray *union_array, const ActivationMapping *mapping)
{
    /* Search for the location of the mapping in the union array */
    gint actual_mapping_index = activation_mapping_index(union_array, mapping);
    
    /* Retrieve the mapping from the union array */
    ActivationMapping *actual_mapping = g_array_index(union_array, ActivationMapping*, actual_mapping_index);
    
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
	g_print("Now activating service: %s of type: %s through: %s\n", actual_mapping->service, actual_mapping->type, target_property);
	g_print("Using arguments: ");
	
	for(i = 0; i < arguments_size; i++)
	    g_print("%s ", arguments[i]);    
	
	g_print("\n");

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
    /* Search for the location of the mapping in the union array */
    gint actual_mapping_index = activation_mapping_index(union_array, mapping);
    
    /* Retrieve the mapping from the union array */
    ActivationMapping *actual_mapping = g_array_index(union_array, ActivationMapping*, actual_mapping_index);
    
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
	g_print("Now deactivating service: %s of type: %s through: %s\n", actual_mapping->service, actual_mapping->type, target_property);
	g_print("Using arguments: ");
	
	for(i = 0; i < arguments_size; i++)
	    g_print("%s ", arguments[i]);
	
	g_print("\n");
	
	if(target_index(target_array, target_property) >= 0) /* Only deactivate services on machines that are available */
	{
	    /* Execute the deactivation operation */	    
	    status = wait_to_finish(exec_deactivate(interface, target_property, actual_mapping->type, arguments, arguments_size, actual_mapping->service));
	}
	else
	{
	    g_print("Skip deactivation of service: %s through: %s\n", actual_mapping->service, target_property);
	    status = 0;
	}
	
	/* Cleanup */
	g_free(arguments);

	if(status != 0)
	    return status; /* If the deactivation fails, abort */
	else
	    actual_mapping->activated = FALSE; /* Mark activation mapping as deactivated */
    }
    
    return 0; /* The deactivation of the closure succeeded */
}

int transition(gchar *interface, GArray *new_activation_mappings, GArray *old_activation_mappings, GArray *target_array)
{
    GArray *union_array;
    GArray *deactivation_array;
    GArray *activation_array;
    unsigned int i;
    int exit_status = 0;
    
    /* Print configurations */
    
    g_print("New configuration mappings:\n");
    print_activation_array(new_activation_mappings);
    
    if(old_activation_mappings != NULL)
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
    else
    {
        union_array = new_activation_mappings;
	    
        for(i = 0; i < union_array->len; i++)
        {
    	    ActivationMapping *mapping = g_array_index(union_array, ActivationMapping*, i);
	    mapping->activated = FALSE;
	}
	    
	deactivation_array = NULL;
	activation_array = new_activation_mappings;
    }

    /* Execute deactivation process */
    
    g_print("Executing deactivation:\n");

    /* Deactivate each mapping closure that is not in the new configuration */
    
    if(deactivation_array != NULL)
    {
        for(i = 0; i < deactivation_array->len; i++)
        {
    	    ActivationMapping *mapping = g_array_index(deactivation_array, ActivationMapping*, i);
	    int status = deactivate(interface, union_array, mapping, target_array);
	    		
	    if(status != 0)
	    {
		/* If the deactivation fails, perform a rollback */
				
		unsigned int j;
		g_print("Deactivation failed! Doing a rollback...\n");
		
		for(j = 0; j < old_activation_mappings->len; j++)
		{
		    ActivationMapping *mapping = g_array_index(union_array, ActivationMapping*, j);
		    
		    if(activate(interface, union_array, mapping) != 0)
			g_print("Rollback failed!\n");		    		    
		}		
		
		exit_status = status;
		break;
	    }
	}
    }

    /* Execute activation process (if deactivation process did not fail) */
    
    if(exit_status == 0)
    {
	g_print("Executing activation:\n");

	/* Activate each mapping closure in the new configuration */
    
	for(i = 0; i < new_activation_mappings->len; i++)
	{
    	    ActivationMapping *mapping = g_array_index(activation_array, ActivationMapping*, i);
	    int status = activate(interface, union_array, mapping);
	    
    	    if(status != 0)
	    {
		/* If the activation fails, perform a rollback */
	    
		unsigned int j;
		g_print("Activation failed! Doing a rollback...\n");
	    
		/* Deactivate the newly activated services */
		for(j = 0; j < activation_array->len; j++)
		{
		    ActivationMapping *mapping = g_array_index(activation_array, ActivationMapping*, j);
	    
		    if(deactivate(interface, union_array, mapping, target_array) != 0)
			g_print("Rollback failed!\n");
		}
	    
		if(old_activation_mappings != NULL)
		{
		    /* Activate all services in the old configuration */
		    for(j = 0; j < old_activation_mappings->len; j++)
		    {
			ActivationMapping *mapping = g_array_index(old_activation_mappings, ActivationMapping*, j);
		
			if(activate(interface, union_array, mapping) != 0)
			    g_print("Rollback failed!\n");
		    }
		}
	    
    		exit_status = status;
		break;
	    }
	}
    }
    
    /* Cleanup */
    
    if(old_activation_mappings != NULL)
    {
	g_array_free(deactivation_array, TRUE);
	g_array_free(activation_array, TRUE);
	g_array_free(union_array, TRUE);
    }
    
    /* Returns the exit status, which is 0 if everything succeeded */
    return exit_status;
}
