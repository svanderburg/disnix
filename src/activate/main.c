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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/wait.h>
#define _GNU_SOURCE
#include <getopt.h>
#include <glib.h>
#include <activationmapping.h>
#include <distributionmapping.h>
#include <defaultoptions.h>
#include <client-interface.h>
#include "transition.h"
#include "locking.h"

static void print_usage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "disnix-activate [--interface interface] [{-p|--profile} profile] [--coordinator-profile-path path] [{-o|--old-manifest} manifest] manifest\n");
    fprintf(stderr, "disnix-activate {-h | --help}\n");
}

static int set_target_profiles(GArray *distribution_array, char *interface, char *profile)
{
    unsigned int i;
    	    
    for(i = 0; i < distribution_array->len; i++)
    {
	int status;
	DistributionItem *item = g_array_index(distribution_array, DistributionItem*, i);
		
	printf("Setting profile: %s on target: %s\n", item->profile, item->target);
	
	status = wait_to_finish(exec_set(interface, item->target, profile, item->profile));
	
	if(status != 0)
	{
	    fprintf(stderr, "Cannot set profile!\n");
	    return FALSE;
	}
    }

    return TRUE;
}

static int set_coordinator_profile(char *coordinator_profile_path, char *distribution_manifest, char *profile, char *username)
{
    gchar *profile_path, *distribution_manifest_path;
    int status;
	    
    printf("Setting the coordinator profile:\n");

    if(coordinator_profile_path == NULL)
	profile_path = g_strconcat(LOCALSTATEDIR, "/nix/profiles/per-user/", username, "/disnix-coordinator", NULL);
    else
	profile_path = g_strdup(coordinator_profile_path);
        
    if(mkdir(profile_path, 0755) == -1)
	fprintf(stderr, "Cannot create profile directory: %s\n", profile_path);
	
    g_free(profile_path);
    
    if((strlen(distribution_manifest) >= 1 && distribution_manifest[0] == '/') ||
       (strlen(distribution_manifest) >= 2 && distribution_manifest[0] == '.' || distribution_manifest[1] == '/'))
        distribution_manifest_path = g_strdup(distribution_manifest);
    else
	distribution_manifest_path = g_strconcat("./", NULL);
    
    if(coordinator_profile_path == NULL)
	profile_path = g_strconcat(LOCALSTATEDIR "/nix/profiles/per-user/", username, "/disnix-coordinator/", profile, NULL);
    else
	profile_path = g_strconcat(coordinator_profile_path, "/", profile, NULL);
    
    status = fork();
    
    if(status == 0)
    {
	char *args[] = {"nix-env", "-p", profile_path, "--set", distribution_manifest_path, NULL};	
	execvp("nix-env", args);
	_exit(1);
    }
    
    g_free(profile_path);
    g_free(distribution_manifest_path);
    
    if(status == -1)
	return FALSE;
    else
    {
	wait(&status);
    
	if(WEXITSTATUS(status) == 0)
	    return TRUE;
	else
	    return FALSE;
    }
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
	{"interface", required_argument, 0, 'i'},
	{"old-manifest", required_argument, 0, 'o'},
	{"coordinator-profile-path", required_argument, 0, 'P'},
	{"profile", required_argument, 0, 'p'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
    };
    char *interface = NULL;
    char *old_manifest = NULL;
    char *profile = NULL;
    char *coordinator_profile_path = NULL;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "o:p:h", long_options, &option_index)) != -1)
    {
	switch(c)
	{
	    case 'i':
		interface = optarg;
		break;
	    case 'o':
	        old_manifest = optarg;
	        break;
	    case 'p':
	        profile = optarg;
		break;
	    case 'P':
	        coordinator_profile_path = optarg;
		break;
	    case 'h':
		print_usage();
		return 0;
	}
    }

    /* Validate options */
    
    interface = check_interface_option(interface);
    profile = check_profile_option(profile);

    if(optind >= argc)
    {
	fprintf(stderr, "A manifest file has to be specified!\n");
	return 1;
    }
    else
    {
	/* Get current username */
	char *username = (getpwuid(geteuid()))->pw_name;
	gchar *old_manifest_file;
	GArray *list_new = create_activation_array(argv[optind]);	
	GArray *list_old;
	GArray *distribution_array = generate_distribution_array(argv[optind]);
	int status;
	
	if(old_manifest == NULL)
        {
	    FILE *file;
	    
	    /* If no old manifest file is given, try to to open the manifest file in the Nix profile */
	    
	    if(coordinator_profile_path == NULL)
		old_manifest_file = g_strconcat(LOCALSTATEDIR "/nix/profiles/per-user/", username, "/disnix-coordinator/", profile, NULL);	    
	    else
		old_manifest_file = g_strconcat(coordinator_profile_path, "/", profile, NULL);
	    
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

	if(old_manifest_file != NULL)
	{	    	    
	    printf("Using previous manifest: %s\n", old_manifest_file);
	    list_old = create_activation_array(old_manifest_file);
	    g_free(old_manifest_file);
        }
	else
	    list_old = NULL;

	/* Try to acquire a lock */
	if(!lock(interface, distribution_array, profile))
	{
	    delete_distribution_array(distribution_array);
	    return 1;
	}
	
	/* Execute transition */	
	status = transition(interface, list_new, list_old);

	/* If the transition failed, abort */
	if(!status)
	{
	    /* Try to release the lock */
	    unlock(interface, distribution_array, profile);
	    
	    delete_distribution_array(distribution_array);
	    return 1;
	}
	
	/* Set the new profiles on the target machines */
	printf("Setting the new profiles on the target machines:\n");
	status = set_target_profiles(distribution_array, interface, profile);

	/* Try to release the lock */
	unlock(interface, distribution_array, profile);
	
	/* If setting the profiles failed, abort */
	if(!status)
	{
	    delete_distribution_array(distribution_array);
	    return 1;
	}
	
	/* Store the activated manifest in the profile of the current user */
	if(!set_coordinator_profile(coordinator_profile_path, argv[optind], profile, username))
	{
	    delete_distribution_array(distribution_array);
	    return 1;
	}

	/* Cleanup */
	delete_distribution_array(distribution_array);

	return 0;
    }
}
