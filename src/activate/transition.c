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

typedef enum
{
    ACTIVATION_ERROR,
    ACTIVATION_IN_PROGRESS,
    ACTIVATION_WAIT,
    ACTIVATION_DONE
}
ActivationStatus;

static ActivationStatus activate(GPtrArray *union_array, const ActivationMappingKey *key, GPtrArray *target_array, const gboolean dry_run, GHashTable *pids)
{
    /* Retrieve the mapping from the union array */
    ActivationMapping *actual_mapping = find_activation_mapping(union_array, key);
    
    /* First, activate all inter-dependency mappings */
    if(actual_mapping->depends_on != NULL)
    {
        unsigned int i;
        ActivationStatus status;
        
        for(i = 0; i < actual_mapping->depends_on->len; i++)
        {
            ActivationMappingKey *dependency = g_ptr_array_index(actual_mapping->depends_on, i);
            status = activate(union_array, dependency, target_array, dry_run, pids);
            
            if(status != ACTIVATION_DONE)
                return status; /* If any of the inter-dependencies has not been activated yet, relay its status */
        }
    }

    /* Finally, activate the mapping itself if it is not activated yet */
    switch(actual_mapping->status)
    {
        case ACTIVATIONMAPPING_DEACTIVATED:
            {
                Target *target = find_target(target_array, actual_mapping->target);
        
                if(request_available_target_core(target)) /* Check if machine has any cores available, if not wait and try again later */
                {
                    gchar **arguments = generate_activation_arguments(target); /* Generate an array of key=value pairs from infrastructure properties */
                    unsigned int arguments_size = g_strv_length(arguments); /* Determine length of the activation arguments array */
                    gchar *interface = find_target_client_interface(target);
                    pid_t pid;
                    
                    print_activation_step(TRUE, actual_mapping, arguments, arguments_size); /* Print debug message */
        
                    if(dry_run)
                        pid = exec_true(); /* Execute dummy process */
                    else
                        pid = exec_activate(interface, actual_mapping->target, actual_mapping->type, arguments, arguments_size, actual_mapping->service); /* Execute the activation operation asynchronously */
                    
                    /* Cleanup */
                    g_strfreev(arguments);
            
                    if(pid == -1)
                    {
                        g_printerr("[target: %s]: Cannot fork activation process of service: %s!\n", actual_mapping->target, actual_mapping->key);
                        return ACTIVATION_ERROR;
                    }
                    else
                    {
                        gint *pidKey = g_malloc(sizeof(gint));
                        *pidKey = pid;
                        
                        actual_mapping->status = ACTIVATIONMAPPING_IN_PROGRESS; /* Mark activation mapping as in progress */
                        g_hash_table_insert(pids, pidKey, actual_mapping); /* Add mapping to the pids table so that we can retrieve its status later */
                        return ACTIVATION_IN_PROGRESS;
                    }
                }
                else
                    return ACTIVATION_WAIT;
             }
        case ACTIVATIONMAPPING_ACTIVATED:
            return ACTIVATION_DONE;
        case ACTIVATIONMAPPING_IN_PROGRESS:
            return ACTIVATION_IN_PROGRESS;
        default:
            return ACTIVATION_ERROR; /* Should never happen */
    }
}

static int deactivate(GPtrArray *union_array, const ActivationMappingKey *key, GPtrArray *target_array, const gboolean dry_run, GHashTable *pids)
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
        ActivationStatus status = deactivate(union_array, (ActivationMappingKey*)dependency_mapping, target_array, dry_run, pids);
    
        if(status != ACTIVATION_DONE)
        {
            g_ptr_array_free(interdependent_mappings, TRUE);
            return status; /* If any inter-dependency is not deactivated, relay its status */
        }
    }
    
    g_ptr_array_free(interdependent_mappings, TRUE);
    
    /* Finally deactivate the mapping itself if it has not been deactivated yet */
    switch(actual_mapping->status)
    {
        case ACTIVATIONMAPPING_ACTIVATED:
            {
                Target *target = find_target(target_array, actual_mapping->target);
        
                if(target == NULL)
                {
                    g_print("[target: %s]: Skip deactivation of service with key: %s deploying package: %s since machine is not present!\n", actual_mapping->key, actual_mapping->target, actual_mapping->service);
                    actual_mapping->status = ACTIVATIONMAPPING_DEACTIVATED;
                    return ACTIVATION_DONE;
                }
                else if(request_available_target_core(target))
                {
                    gchar **arguments = generate_activation_arguments(target); /* Generate an array of key=value pairs from infrastructure properties */
                    unsigned int arguments_size = g_strv_length(arguments); /* Determine length of the activation arguments array */
                    gchar *interface = find_target_client_interface(target);
                    pid_t pid;
                    
                    print_activation_step(FALSE, actual_mapping, arguments, arguments_size); /* Print debug message */
            
                    if(dry_run)
                        pid = exec_true(); /* Execute dummy process */
                    else
                        pid = exec_deactivate(interface, actual_mapping->target, actual_mapping->type, arguments, arguments_size, actual_mapping->service); /* Execute the deactivation operation asynchronously */
                    
                    /* Cleanup */
                    g_free(arguments);
            
                    if(pid == -1)
                    {
                        g_printerr("[target: %s]: Cannot fork deactivation process of service: %s!\n", actual_mapping->target, actual_mapping->key);
                        return ACTIVATION_ERROR;
                    }
                    else
                    {
                        gint *pidKey = g_malloc(sizeof(gint));
                        *pidKey = pid;
                        
                        actual_mapping->status = ACTIVATIONMAPPING_IN_PROGRESS; /* Mark activation mapping as in progress */
                        g_hash_table_insert(pids, pidKey, actual_mapping); /* Add mapping to the pids table so that we can retrieve its status later */
                        return ACTIVATION_IN_PROGRESS;
                    }
                }
                else
                    return ACTIVATION_WAIT;
           }
        case ACTIVATIONMAPPING_DEACTIVATED:
            return ACTIVATION_DONE;
        case ACTIVATIONMAPPING_IN_PROGRESS:
            return ACTIVATION_IN_PROGRESS;
        default:
            return ACTIVATION_ERROR; /* Should never happen */
    }
}

static void wait_for_activation_or_deactivation(const int activate, GHashTable *pids, GPtrArray *target_array)
{
    int status;
    Target *target;
    
    /* Wait for an activation/deactivation process to finish */
    pid_t pid = wait(&status);
    
    if(pid != -1)
    {
        /* Find the corresponding activation mapping and remove it from the pids table */
        ActivationMapping *mapping = g_hash_table_lookup(pids, &pid);
        g_hash_table_remove(pids, &pid);
    
        /* Change the status of the mapping, if the process returns success */
        if(WEXITSTATUS(status) == 0)
        {
            if(activate)
                mapping->status = ACTIVATIONMAPPING_ACTIVATED;
            else
                mapping->status = ACTIVATIONMAPPING_DEACTIVATED;
        }
        else
        {
            mapping->status = ACTIVATIONMAPPING_ERROR;
        
            if(activate)
                g_printerr("[target: %s]: Activation failed of service: %s\n", mapping->target, mapping->key);
            else
                g_printerr("[target: %s]: Deactivation failed of service: %s\n", mapping->target, mapping->key);
        }
    
        /* Signal the target to make the CPU core available again */
        target = find_target(target_array, mapping->target);
        signal_available_target_core(target);
    }
}

static void destroy_pids_key(gpointer data)
{
    gint *key = (gint*)data;
    g_free(key);
}

static int iterate_over_activation_or_deactivation_mappings(const int activate_flag, GPtrArray *mappings, GPtrArray *union_array, GPtrArray *target_array, const gboolean dry_run)
{
    GHashTable *pids = g_hash_table_new_full(g_int_hash, g_int_equal, destroy_pids_key, NULL);
    unsigned int numDone = 0;
    int success = TRUE;
    
    do
    {
        unsigned int i;
        numDone = 0;
        
        for(i = 0; i < mappings->len; i++)
        {
            ActivationMapping *mapping = g_ptr_array_index(mappings, i);
            ActivationStatus status;
            
            if(activate_flag)
                status = activate(union_array, (ActivationMappingKey*)mapping, target_array, dry_run, pids);
            else
                status = deactivate(union_array, (ActivationMappingKey*)mapping, target_array, dry_run, pids);
            
            if(status == ACTIVATION_ERROR)
            {
                success = FALSE;
                numDone++;
            }
            else if(status == ACTIVATION_DONE)
                numDone++;
            
            wait_for_activation_or_deactivation(activate_flag, pids, target_array);
        }
    }
    while(numDone < mappings->len);
    
    g_hash_table_destroy(pids);
    return success;
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

static void rollback_to_old_mappings(GPtrArray *union_array, GPtrArray *old_activation_mappings, GPtrArray *target_array, const gboolean dry_run)
{
    mark_erroneous_mappings(union_array, ACTIVATIONMAPPING_ACTIVATED); /* Mark erroneous mappings as activated */
    if(!iterate_over_activation_or_deactivation_mappings(TRUE, old_activation_mappings, union_array, target_array, dry_run)) {
        g_print("[coordinator]: Rollback failed!\n\n");
    }
}

static int deactivate_obsolete_mappings(GPtrArray *deactivation_array, GPtrArray *union_array, GPtrArray *target_array, GPtrArray *old_activation_mappings, const gboolean dry_run)
{
    g_print("[coordinator]: Executing deactivation of services:\n");

    if(deactivation_array == NULL)
        return 0;
    else
    {
        if(iterate_over_activation_or_deactivation_mappings(FALSE, deactivation_array, union_array, target_array, dry_run))
            return 0;
        else
        {
            /* If the deactivation fails, perform a rollback */
            g_print("[coordinator]: Deactivation failed! Doing a rollback...\n");
            rollback_to_old_mappings(union_array, old_activation_mappings, target_array, dry_run);
            return 1;
        }
    }
}

static void rollback_new_mappings(GPtrArray *activation_array, GPtrArray *union_array, GPtrArray *target_array, const gboolean dry_run)
{
    mark_erroneous_mappings(union_array, ACTIVATIONMAPPING_DEACTIVATED); /* Mark erroneous mappings as deactivated */
    if(!iterate_over_activation_or_deactivation_mappings(FALSE, activation_array, union_array, target_array, dry_run))
    {
        g_printerr("[coordinator]: Rollback failed!\n\n");
        g_printerr("The system is now in an inconsistent state! Before redeploying it again, please\n");
        g_printerr("investigate the errors and correct them!\n\n");
    }
}

static int activate_new_mappings(GPtrArray *activation_array, GPtrArray *union_array, GPtrArray *target_array, GPtrArray *old_activation_mappings, const gboolean dry_run)
{
    g_print("[coordinator]: Executing activation of services:\n");
    
    if(iterate_over_activation_or_deactivation_mappings(TRUE, activation_array, union_array, target_array, dry_run))
        return 0;
    else
    {
        /* If the activation fails, perform a rollback */
        g_printerr("[coordinator]: Activation failed! Doing a rollback...\n");
        
        rollback_new_mappings(activation_array, union_array, target_array, dry_run);
        
        if(old_activation_mappings != NULL)
            rollback_to_old_mappings(union_array, old_activation_mappings, target_array, dry_run);
        
        return 1;
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

int transition(GPtrArray *new_activation_mappings, GPtrArray *old_activation_mappings, GPtrArray *target_array, const gboolean dry_run)
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
    if((status = deactivate_obsolete_mappings(deactivation_array, union_array, target_array, old_activation_mappings, dry_run)) != 0)
    {
        cleanup(old_activation_mappings, deactivation_array, activation_array, union_array);
        return status;
    }

    /* Execute activation process (if deactivation process did not fail) */
    if((status = activate_new_mappings(activation_array, union_array, target_array, old_activation_mappings, dry_run)) != 0)
    {
        cleanup(old_activation_mappings, deactivation_array, activation_array, union_array);
        return status;
    }
    
    /* Cleanup */
    cleanup(old_activation_mappings, deactivation_array, activation_array, union_array);
    
    /* Returns the exit status, which is 0 if everything succeeded */
    return 0;
}
