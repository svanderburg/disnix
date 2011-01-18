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

#include <stdio.h>
#include <getopt.h>
#define _GNU_SOURCE
#include <defaultoptions.h>
#include "activate.h"

static void print_usage(char *command)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s [options] manifest\n\n", command);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "--interface interface\n");
    fprintf(stderr, "--profile profile\n");
    fprintf(stderr, "--coordinator-profile-path path\n");
    fprintf(stderr, "--old-manifest manifest\n");
    fprintf(stderr, "--target-property targetProperty\n");
    fprintf(stderr, "--no-coordinator-profile\n");
    fprintf(stderr, "--no-target-profiles\n");
    fprintf(stderr, "\n");
    
    fprintf(stderr, "%s {-h | --help}\n", command);
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
	{"interface", required_argument, 0, 'I'},
	{"old-manifest", required_argument, 0, 'o'},
	{"coordinator-profile-path", required_argument, 0, 'P'},
	{"profile", required_argument, 0, 'p'},
	{"no-coordinator-profile", no_argument, 0, 'c'},
	{"no-target-profiles", no_argument, 0, 'C'},
	{"no-upgrade", no_argument, 0, 'u'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
    };
    char *interface = NULL;
    char *old_manifest = NULL;
    char *profile = NULL;
    char *coordinator_profile_path = NULL;
    int no_coordinator_profile = FALSE;
    int no_target_profiles = FALSE;
    int no_upgrade = FALSE;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "o:p:h", long_options, &option_index)) != -1)
    {
	switch(c)
	{
	    case 'I':
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
	    case 'c':
		no_coordinator_profile = TRUE;
		break;
	    case 'C':
		no_target_profiles = TRUE;
		break;
	    case 'u':
		no_upgrade = TRUE;
		break;
	    case 'h':
		print_usage(argv[0]);
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
	return activate_system(interface, argv[optind], old_manifest, coordinator_profile_path, profile, no_coordinator_profile, no_target_profiles, no_upgrade); /* Execute activation operation */
}
