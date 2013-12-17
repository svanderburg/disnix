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

#include "profiles.h"
#include <distributionmapping.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

int set_target_profiles(const GArray *distribution_array, gchar *interface, gchar *profile)
{
    unsigned int i;
    int exit_status = 0;
    
    for(i = 0; i < distribution_array->len; i++)
    {
	int status;
	DistributionItem *item = g_array_index(distribution_array, DistributionItem*, i);
		
	g_print("Setting profile: %s on target: %s\n", item->profile, item->target);
	
	status = wait_to_finish(exec_set(interface, item->target, profile, item->profile));
	
	if(status != 0)
	{
	    g_printerr("Cannot set profile!\n");
	    exit_status = status;
	}
    }

    return exit_status;
}

int set_coordinator_profile(const gchar *coordinator_profile_path, const gchar *manifest_file, const gchar *profile, const gchar *username)
{
    gchar *profile_path, *manifest_file_path;
    int status;
	    
    g_print("Setting the coordinator profile:\n");

    /* Determine which profile path to use, if a coordinator profile path is given use this value otherwise the default */
    if(coordinator_profile_path == NULL)
	profile_path = g_strconcat(LOCALSTATEDIR, "/nix/profiles/per-user/", username, "/disnix-coordinator", NULL);
    else
	profile_path = g_strdup(coordinator_profile_path);
    
    /* Create the profile directory */
    if(mkdir(profile_path, 0755) == -1)
    {
	if(errno != EEXIST)
	    g_printerr("Cannot create profile directory: %s\n", profile_path);
    }
    
    /* Profile path is not needed anymore */
    g_free(profile_path);
    
    /* If the manifest file is an absolute path or a relative path starting
     * with ./ then the path is OK
     */
     
    if((strlen(manifest_file) >= 1 && manifest_file[0] == '/') ||
       (strlen(manifest_file) >= 2 && manifest_file[0] == '.' || manifest_file[1] == '/'))
        manifest_file_path = g_strdup(manifest_file);
    else
	manifest_file_path = g_strconcat("./", manifest_file, NULL); /* Otherwise add ./ in front of the path */
    
    /* Determine the path to the profile */
    if(coordinator_profile_path == NULL)
	profile_path = g_strconcat(LOCALSTATEDIR "/nix/profiles/per-user/", username, "/disnix-coordinator/", profile, NULL);
    else
	profile_path = g_strconcat(coordinator_profile_path, "/", profile, NULL);
    
    /* Execute nix-env --set operation to change the coordinator profile so
     * that the new configuration is known
     */
     
    status = fork();
    
    if(status == 0)
    {
	char *const args[] = {"nix-env", "-p", profile_path, "--set", manifest_file_path, NULL};
	execvp("nix-env", args);
	_exit(1);
    }
    
    /* Cleanup */
    g_free(profile_path);
    g_free(manifest_file_path);
    
    /* If the process suceeds the the operation succeeded */
    if(status == -1)
	return -1;
    else
    {
	wait(&status);
    
	if(WEXITSTATUS(status) == 0)
	    return 0;
	else
	    return WEXITSTATUS(status);
    }
}
