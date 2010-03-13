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
#include "activationmapping.h"
#include "distributionmapping.h"

static void print_usage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "disnix-activate [--interface interface] [{-p|--profile} profile] [{-o|--old-manifest} manifest] manifest\n");
    fprintf(stderr, "disnix-activate {-h | --help}\n");
}

static int activate(GArray *union_list, ActivationMapping *mapping, gchar *interface)
{
    gint actual_mapping_index = activation_mapping_index(union_list, mapping);
    ActivationMapping *actual_mapping = g_array_index(union_list, ActivationMapping*, actual_mapping_index);
    
    /* First activate all inter-dependencies */
    if(actual_mapping->depends_on != NULL)
    {
	unsigned int i;
	
	for(i = 0; i < actual_mapping->depends_on->len; i++)
	{
	    Dependency *dependency = g_array_index(actual_mapping->depends_on, Dependency*, i);
	    ActivationMapping lookup;
	    
	    lookup.service = dependency->service;
	    lookup.target = dependency->target;	    	    	    
	    
	    if(!activate(union_list, &lookup, interface))
		return FALSE;
	}
    }
    
    /* Finally activate the service itself */
    if(!actual_mapping->activated)
    {
	gchar **arguments = generate_activation_arguments(actual_mapping->target);
	unsigned int arguments_size = g_strv_length(arguments);
	gchar *target_interface = get_target_interface(actual_mapping);
	int status, i;
	
	printf("Now activating service: %s of type: %s through: %s\n", actual_mapping->service, actual_mapping->type, target_interface);
	printf("Using arguments: ");
	
	for(i = 0; i < arguments_size; i++)
	    printf("%s ", arguments[i]);    
	
	printf("\n");

	status = fork();
	
	if(status == 0)
	{
	    unsigned int i;
	    char **args = (char**)g_malloc((8 + 2 * arguments_size) * sizeof(gchar*));
	    
	    args[0] = interface;
	    args[1] = "--activate";
	    args[2] = "--type";
	    args[3] = actual_mapping->type;
	    args[4] = "--target";
	    args[5] = target_interface;
	    
	    for(i = 0; i < arguments_size * 2; i += 2)
	    {
		args[i + 6] = "--arguments";
		args[i + 7] = arguments[i / 2];
	    }
	    
	    args[i + 6] = actual_mapping->service;
	    args[i + 7] = NULL;
	    
	    execvp(interface, args);
	    _exit(1);	    
	}
	
	g_strfreev(arguments);
	
	if(status == -1)
	    return FALSE;
	else
	{	    	
	    wait(&status);
		
	    if(WEXITSTATUS(status) != 0)
		return FALSE;
	}
	
	actual_mapping->activated = TRUE;
    }
    
    return TRUE;
}

static int deactivate(GArray *union_list, ActivationMapping *mapping, gchar *interface)
{
    gint actual_mapping_index = activation_mapping_index(union_list, mapping);
    ActivationMapping *actual_mapping = g_array_index(union_list, ActivationMapping*, actual_mapping_index);
    GArray *interdependent_services = find_interdependent_mappings(union_list, actual_mapping);
    
    /* First deactivate all service which have an inter-dependency on this service */
        
    unsigned int i;
	
    for(i = 0; i < interdependent_services->len; i++)
    {
        ActivationMapping *dependency_mapping = g_array_index(interdependent_services, ActivationMapping*, i);
        if(!deactivate(union_list, dependency_mapping, interface))
	    return FALSE;
    }
    
    /* Finally deactivate the service itself */
    if(actual_mapping->activated)
    {
	gchar **arguments = generate_activation_arguments(actual_mapping->target);
	unsigned int arguments_size = g_strv_length(arguments);
	gchar *target_interface = get_target_interface(actual_mapping);
	int status, i;
	
	printf("Now deactivating service: %s of type: %s through: %s\n", actual_mapping->service, actual_mapping->type, target_interface);
	printf("Using arguments: ");
	
	for(i = 0; i < arguments_size; i++)
	    printf("%s ", arguments[i]);
	
	printf("\n");
	
	status = fork();
	
	if(status == 0)
	{
	    unsigned int i;
	    char **args = (char**)g_malloc((8 + 2 * arguments_size) * sizeof(gchar*));
	    
	    args[0] = interface;
	    args[1] = "--deactivate";
	    args[2] = "--type";
	    args[3] = actual_mapping->type;
	    args[4] = "--target";
	    args[5] = target_interface;
	    
	    for(i = 0; i < arguments_size * 2; i += 2)
	    {
	    	args[i + 6] = "--arguments";
		args[i + 7] = arguments[i / 2];
	    }
	    
	    args[i + 6] = actual_mapping->service;
	    args[i + 7] = NULL;
	    
	    execvp(interface, args);
	    _exit(1);
	}
		
	g_free(arguments);

	if(status == -1)
	    return FALSE;
	else
	{
	    wait(&status);
	
	    if(WEXITSTATUS(status) != 0)
		return FALSE;
	}
	
	actual_mapping->activated = FALSE;
    }
    
    return TRUE;
}

static int transition(GArray *list_new, GArray *list_old, char *interface)
{
    GArray *unio;
    GArray *deactivate_list;
    GArray *activate_list;
    unsigned int i;        
    
    printf("new:\n");
    print_activation_array(list_new);
    
    if(list_old != NULL)
    {
        printf("old:\n");
        print_activation_array(list_old);

        printf("intersect:\n");
        GArray *intsect = intersect_activation_array(list_new, list_old);
        print_activation_array(intsect);
		    	    
        printf("to deactivate:\n");
        deactivate_list = substract_activation_array(list_old, intsect);
        print_activation_array(deactivate_list);
	    
        printf("to activate:\n");
        activate_list = substract_activation_array(list_new, intsect);
        print_activation_array(activate_list);

        printf("union:\n");
	unio = union_activation_array(list_old, list_new, intsect);
	print_activation_array(unio);
    }	
    else
    {	    
        unio = list_new;
	    
        for(i = 0; i < unio->len; i++)
        {
    	    ActivationMapping *mapping = g_array_index(unio, ActivationMapping*, i);
	    mapping->activated = FALSE;
	}
	    
	deactivate_list = NULL;
	activate_list = list_new;
    }

    printf("Deactivate:\n");
	
    if(deactivate_list != NULL)
    {
        for(i = 0; i < deactivate_list->len; i++)
        {
    	    ActivationMapping *mapping = g_array_index(deactivate_list, ActivationMapping*, i);
				
	    if(!deactivate(unio, mapping, interface))
	    {
		unsigned int j;
		printf("Deactivation failed! Doing a rollback...\n");
		
		for(j = 0; j < list_old->len; j++)
		{
		    ActivationMapping *mapping = g_array_index(unio, ActivationMapping*, j);
		    
		    if(!activate(unio, mapping, interface))
			printf("Rollback failed!\n");		    		    
		}		
		
		return FALSE;
	    }
	}
    }
	
    printf("Activate:\n");
		
    for(i = 0; i < activate_list->len; i++)
    {
        ActivationMapping *mapping = g_array_index(activate_list, ActivationMapping*, i);
	    
        if(!activate(unio, mapping, interface))
	{
	    unsigned int j;
	    printf("Activation failed! Doing a rollback...\n");
	    
	    /* Deactivate the newly activated services */
	    for(j = 0; j < activate_list->len; j++)
	    {
		ActivationMapping *mapping = g_array_index(activate_list, ActivationMapping*, j);
	    
		if(!deactivate(unio, mapping, interface))
		    printf("Rollback failed!\n");
	    }
	    
	    if(list_old != NULL)
	    {
		/* Activate all services in the old configuration */
		for(j = 0; j < list_old->len; j++)
		{
		    ActivationMapping *mapping = g_array_index(list_old, ActivationMapping*, j);
		
		    if(!activate(unio, mapping, interface))
			printf("Rollback failed!\n");
		}
	    }
	    
    	    return FALSE;
	}
    }
    
    return TRUE;
}

static int set_target_profiles(GArray *distribution_array, char *interface, char *profile)
{
    unsigned int i;
    	    
    for(i = 0; i < distribution_array->len; i++)
    {
	int status;
	DistributionItem *item = g_array_index(distribution_array, DistributionItem*, i);
		
	printf("Setting profile: %s on target: %s\n", item->profile, item->target);
	
	status = fork();
	
	if(status == 0)
	{
	    char *args[] = {interface, "--target", item->target, "--profile", profile, "--set", item->profile, NULL};
	    execvp(interface, args);
	    _exit(1);
	}
	
	if(status == -1)
	    return FALSE;
	else
	{
	    wait(&status);
	    
	    if(WEXITSTATUS(status) != 0)
		return FALSE;
	}
    }

    return TRUE;
}

static int set_coordinator_profile(char *distribution_manifest, char *profile, char *username)
{
    gchar *profile_path, *distribution_manifest_path;
    int status;
	    
    printf("Setting the coordinator profile:\n");
	    
    profile_path = g_strconcat(LOCALSTATEDIR, "/nix/profiles/per-user/", username, "/disnix-coordinator", NULL);
    mkdir(profile_path, 0755);
    g_free(profile_path);
    
    if((strlen(distribution_manifest) >= 1 && distribution_manifest[0] == '/') ||
       (strlen(distribution_manifest) >= 2 && distribution_manifest[0] == '.' || distribution_manifest[1] == '/'))
        distribution_manifest_path = g_strdup(distribution_manifest);
    else
	distribution_manifest_path = g_strconcat("./", NULL);
    
    profile_path = g_strconcat(LOCALSTATEDIR "/nix/profiles/per-user/", username, "/disnix-coordinator/", profile, NULL);
    
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

static int unlock(GArray *distribution_array, char *interface, char *profile)
{
    unsigned int i, running_processes = 0;
    int exit_status = TRUE;
    int status;
    
    /* For each locked machine, release the lock */
    for(i = 0; i < distribution_array->len; i++)
    {
	DistributionItem *item = g_array_index(distribution_array, DistributionItem*, i);
	status = fork();
	
	if(status == -1)
	{
	    fprintf(stderr, "Error with forking unlock process!\n");
	    exit_status = FALSE;
	}
	else if(status == 0)
	{
	    char *args[] = {interface, "--unlock", "--target", item->target, "--profile", profile, NULL};
	    execvp(interface, args);
	    _exit(1);
	}
	else
	    running_processes++;
    }
    
    /* Wait until every lock is released */
    for(i = 0; i < running_processes; i++)
    {
        wait(&status);
	    
        if(WEXITSTATUS(status) != 0)
	{
    	    fprintf(stderr, "Failed to release the lock!\n");
	    exit_status = FALSE;
	}
    }
    
    /* Return exit status */
    return exit_status;
}

static int lock(GArray *distribution_array, char *interface, char *profile)
{
    unsigned int i;
    GArray *try_array = g_array_new(FALSE, FALSE, sizeof(DistributionItem*));
    GArray *lock_array = g_array_new(FALSE, FALSE, sizeof(DistributionItem*));
    int exit_status = TRUE;
    int status;
    
    /* For each machine acquire a lock */
    for(i = 0; i < distribution_array->len; i++)
    {
	DistributionItem *item = g_array_index(distribution_array, DistributionItem*, i);
	status = fork();

	if(status == -1)
	{
	    fprintf(stderr, "Error with forking lock process!\n");
	    exit_status = FALSE;
	}	
	else if(status == 0)
	{
	    char *args[] = {interface, "--lock", "--target", item->target, "--profile", profile, NULL};
	    execvp(interface, args);
	    _exit(1);
	}
	else
	    g_array_append_val(try_array, item);
    }    
    
    /* Wait until every lock is acquired */
    for(i = 0; i < try_array->len; i++)
    {
	wait(&status);
	
	if(WEXITSTATUS(status) == 0)
	    g_array_append_val(lock_array, g_array_index(try_array, DistributionItem*, i));
	else
	{
	    fprintf(stderr, "Failed to acquire a lock!\n");
	    exit_status = FALSE;
	}
    }
    
    /* If a lock fails then unlock every machine that is locked */
    if(!exit_status)
	unlock(lock_array, interface, profile);
    
    /* Cleanup */
    g_array_free(try_array, TRUE);
    g_array_free(lock_array, TRUE);
    
    /* Return exit status */
    return exit_status;
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
	{"interface", required_argument, 0, 'i'},
	{"old-manifest", required_argument, 0, 'o'},
	{"profile", required_argument, 0, 'p'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
    };
    char *interface = NULL;
    char *old_manifest = NULL;
    char *profile = "default";
    
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
	    case 'h':
		print_usage();
		return 0;
	}
    }

    /* Validate options */
    if(interface == NULL)
    {
	char *interface_env = getenv("DISNIX_CLIENT_INTERFACE");
	
	if(interface_env != NULL)
	    interface = interface_env;
	else
	    interface = "disnix-client";
    }

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
	    FILE *file = fopen(old_manifest_file, "r");
	    
	    /* If no old manifest file is given, try to to open the manifest file in the Nix profile */	    
	    old_manifest_file = g_strconcat(LOCALSTATEDIR "/nix/profiles/per-user/", username, "/disnix-coordinator/", profile, NULL);	    
	    
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
	if(!lock(distribution_array, interface, profile))
	{
	    delete_distribution_array(distribution_array);
	    return 1;
	}
	
	/* Execute transition */	
	status = transition(list_new, list_old, interface);

	/* If the transition failed, abort */
	if(!status)
	{
	    /* Try to release the lock */
	    unlock(distribution_array, interface, profile);
	    
	    delete_distribution_array(distribution_array);
	    return 1;
	}
	
	/* Set the new profiles on the target machines */
	printf("Setting the new profiles on the target machines:\n");
	status = set_target_profiles(distribution_array, interface, profile);

	/* Try to release the lock */
	unlock(distribution_array, interface, profile);
	
	/* If setting the profiles failed, abort */
	if(!status)
	{
	    delete_distribution_array(distribution_array);
	    return 1;
	}
	
	/* Store the activated manifest in the profile of the current user */
	if(!set_coordinator_profile(argv[optind], profile, username))
	{
	    delete_distribution_array(distribution_array);
	    return 1;
	}

	/* Cleanup */
	delete_distribution_array(distribution_array);

	return 0;
    }
}
