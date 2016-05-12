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

#include "profiles.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <pwd.h>
#include <errno.h>
#include <string.h>

#include <distributionmapping.h>
#include <targets.h>
#include <client-interface.h>

#define RESOLVED_PATH_MAX_SIZE 4096

static int set_target_profiles(const GPtrArray *distribution_array, const GPtrArray *target_array, gchar *profile)
{
    unsigned int i;
    int exit_status = 0, running_processes = 0;
    
    for(i = 0; i < distribution_array->len; i++)
    {
        pid_t pid;
        DistributionItem *item = g_ptr_array_index(distribution_array, i);
        Target *target = find_target(target_array, item->target);
        
        g_print("[target: %s]: Setting Disnix profile: %s\n", item->target, item->profile);
        
        pid = exec_set(target->client_interface, item->target, profile, item->profile);
        
        if(pid == -1)
        {
            g_print("[target: %s]: Error forking nix-env --set process!\n", item->target);
            exit_status = -1;
        }
        else
            running_processes++;
    }
    
    /* Check statusses of the running processes */
    for(i = 0; i < running_processes; i++)
    {
        int status = wait_to_finish(0);

        /* If one of the processes fail, change the exit status */
        if(status != 0)
        {
            g_printerr("Cannot set profile!\n");
            exit_status = status;
        }
    }

    return exit_status;
}

static int set_coordinator_profile(const gchar *coordinator_profile_path, const gchar *manifest_file, const gchar *profile)
{
    gchar *profile_path, *manifest_file_path;
    char resolved_path[RESOLVED_PATH_MAX_SIZE];
    ssize_t resolved_path_size;
    
    /* Get current username */
    char *username = (getpwuid(geteuid()))->pw_name;

    /* Determine which profile path to use, if a coordinator profile path is given use this value otherwise the default */
    if(coordinator_profile_path == NULL)
        profile_path = g_strconcat(LOCALSTATEDIR, "/nix/profiles/per-user/", username, "/disnix-coordinator", NULL);
    else
        profile_path = g_strdup(coordinator_profile_path);
    
    /* Create the profile directory */
    if(mkdir(profile_path, 0755) == -1 && errno != EEXIST)
        g_printerr("[coordinator]: Cannot create profile directory: %s\n", profile_path);
    
    /* Profile path is not needed anymore */
    g_free(profile_path);
    
    /* If the manifest file is an absolute path or a relative path starting
     * with ./ then the path is OK
     */
     
    if((strlen(manifest_file) >= 1 && manifest_file[0] == '/') ||
       (strlen(manifest_file) >= 2 && (manifest_file[0] == '.' || manifest_file[1] == '/')))
        manifest_file_path = g_strdup(manifest_file);
    else
        manifest_file_path = g_strconcat("./", manifest_file, NULL); /* Otherwise add ./ in front of the path */
    
    /* Determine the path to the profile */
    if(coordinator_profile_path == NULL)
        profile_path = g_strconcat(LOCALSTATEDIR "/nix/profiles/per-user/", username, "/disnix-coordinator/", profile, NULL);
    else
        profile_path = g_strconcat(coordinator_profile_path, "/", profile, NULL);
    
    /* Resolve the manifest file to which the coordinator profile points */
    
    resolved_path_size = readlink(profile_path, resolved_path, RESOLVED_PATH_MAX_SIZE);
    
    if(resolved_path_size != -1 && (strlen(profile_path) != resolved_path_size || strncmp(resolved_path, profile_path, resolved_path_size) != 0)) /* If the symlink resolves not to itself, we get a generation symlink that we must resolve again */
    {
        gchar *generation_path;
        
        resolved_path[resolved_path_size] = '\0';
        
        if(coordinator_profile_path == NULL)
            generation_path = g_strconcat(LOCALSTATEDIR "/nix/profiles/per-user/", username, "/disnix-coordinator/", resolved_path, NULL);
        else
            generation_path = g_strconcat(coordinator_profile_path, "/", resolved_path, NULL);
        
        resolved_path_size = readlink(generation_path, resolved_path, RESOLVED_PATH_MAX_SIZE);
        
        g_free(generation_path);
    }
    
    if(resolved_path_size == -1 || (strlen(manifest_file) == resolved_path_size && strncmp(resolved_path, manifest_file, resolved_path_size) != 0)) /* Only configure the configurator profile if the given manifest is not identical to the previous manifest */
    {
        /* Execute nix-env --set operation to change the coordinator profile so
         * that the new configuration is known
         */
         
        int status = fork();
    
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
            return WEXITSTATUS(status);
        }
    }
    else
        return 0;
}

static void cleanup(GPtrArray *target_array, GPtrArray *distribution_array)
{
    delete_target_array(target_array);
    delete_distribution_array(distribution_array);
}

int set_profiles(const gchar *manifest_file, const gchar *coordinator_profile_path, char *profile, const int no_coordinator_profile, const int no_target_profiles)
{
    GPtrArray *distribution_array = generate_distribution_array(manifest_file);
    GPtrArray *target_array = generate_target_array(manifest_file);
    int status;
    
    if(!no_target_profiles && (status = set_target_profiles(distribution_array, target_array, profile)) != 0)
    {
        cleanup(target_array, distribution_array);
        return status;
    }
    
    if(!no_coordinator_profile && (status = set_coordinator_profile(coordinator_profile_path, manifest_file, profile)) != 0)
    {
        cleanup(target_array, distribution_array);
        return status;
    }
    
    cleanup(target_array, distribution_array);
    return 0;
}
