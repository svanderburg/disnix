/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2010  Sander van der Burg
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

#include "activate.h"
#include "locking.h"
#include "transition.h"
#include "profiles.h"

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdio.h>

#include <distributionmapping.h>
#include <activationmapping.h>
#include <infrastructure.h>

int activate_system(gchar *interface, gchar *infrastructure, gchar *new_manifest, gchar *old_manifest, gchar *coordinator_profile_path, gchar *profile, gchar *target_property, gboolean no_coordinator_profile)
{
    gchar *old_manifest_file;
    GArray *old_activation_mappings;
    int status;
    int exit_status = 0;
    
    /* Get all the distribution items of the new configuration */
    GArray *distribution_array = generate_distribution_array(new_manifest);

    /* Get all the activation items of the new configuration */
    GArray *new_activation_mappings = create_activation_array(new_manifest);
    
    /* Get all target properties from the infrastructure model */
    GArray *target_array = create_target_array(infrastructure, target_property);
    
    /* Get current username */
    char *username = (getpwuid(geteuid()))->pw_name;

    /* If no previous configuration is given, check whether we have one in the coordinator profile */
    if(old_manifest == NULL)
    {
        FILE *file;
	    
        if(coordinator_profile_path == NULL)
    	    old_manifest_file = g_strconcat(LOCALSTATEDIR "/nix/profiles/per-user/", username, "/disnix-coordinator/", profile, NULL);	    
	else
	    old_manifest_file = g_strconcat(coordinator_profile_path, "/", profile, NULL);
	    
	/* Try to open file => if it succeeds we have a previous configuration */
	file = fopen(old_manifest_file, "r");
	    
	if(file == NULL)
	{
	    g_free(old_manifest_file);
	    old_manifest_file = NULL;
	}
	else
	    fclose(file);
    }
    else
        old_manifest_file = g_strdup(old_manifest);

    /* If we have an old configuration -> open it */
    if(old_manifest_file != NULL)
    {	    	    
        g_print("Using previous manifest: %s\n", old_manifest_file);
        old_activation_mappings = create_activation_array(old_manifest_file);
        
	/* Free the variable because it's not needed anymore */
	g_free(old_manifest_file);
    }
    else
        old_activation_mappings = NULL;

    /* Try to acquire a lock */
    
    status = lock(interface, distribution_array, profile);
    
    if(status == 0)
    {
	/* Execute transition */
	status = transition(interface, new_activation_mappings, old_activation_mappings, target_array);
	
	if(status == 0)
	{	    	    
	    /* Set the new profiles on the target machines */
	    g_print("Setting the new profiles on the target machines:\n");
	    status = set_target_profiles(distribution_array, interface, profile);
	    
	    /* Try to release the lock */
	    unlock(interface, distribution_array, profile);
	    
	    /* If setting the profiles succeeds -> set the coordinator profile */
	    if(status == 0 && !no_coordinator_profile)
	    {
		status = set_coordinator_profile(coordinator_profile_path, new_manifest, profile, username);
		
		if(status != 0)
		    exit_status = status; /* if settings the coordinator profile fails -> change exit status */
	    }
	    else
		exit_status = status; /* else change exit status */
	}
	else
	{
	    /* Try to release the lock */
	    unlock(interface, distribution_array, profile); 
	    exit_status = status;
	}
    }
    else
	exit_status = status;

    /* Cleanup */
    delete_target_array(target_array);
    delete_distribution_array(distribution_array);
    delete_activation_array(new_activation_mappings);
    
    if(old_activation_mappings != NULL)
	delete_activation_array(old_activation_mappings);
    
    /* Return the exit status, which is 0 if everything succeeded */
    return exit_status;
}
