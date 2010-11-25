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
    fprintf(stderr, "%s [--interface interface] [{-p|--profile} profile] [--coordinator-profile-path path] [{-o|--old-manifest} manifest] [--target-property targetProperty] --infrastructure infrastructure_expr manifest\n", command);
    fprintf(stderr, "%s {-h | --help}\n", command);
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
	{"infrastructure", required_argument, 0, 'i'},
	{"interface", required_argument, 0, 'I'},
	{"old-manifest", required_argument, 0, 'o'},
	{"coordinator-profile-path", required_argument, 0, 'P'},
	{"profile", required_argument, 0, 'p'},
	{"target-property", required_argument, 0, 't'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
    };
    char *infrastructure = NULL;
    char *interface = NULL;
    char *old_manifest = NULL;
    char *profile = NULL;
    char *coordinator_profile_path = NULL;
    char *target_property = NULL;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "i:o:p:h", long_options, &option_index)) != -1)
    {
	switch(c)
	{
	    case 'i':
		infrastructure = optarg;
		break;
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
	    case 't':
		target_property = optarg;
		break;
	    case 'h':
		print_usage(argv[0]);
		return 0;
	}
    }

    /* Validate options */
    
    interface = check_interface_option(interface);
    profile = check_profile_option(profile);
    target_property = check_target_property_option(target_property);
    
    if(infrastructure == NULL)
    {
	fprintf(stderr, "An infrastructure expression has to be specified!\n");
	return 1;
    }

    if(optind >= argc)
    {
	fprintf(stderr, "A manifest file has to be specified!\n");
	return 1;
    }
    else
	return activate_system(interface, infrastructure, argv[optind], old_manifest, coordinator_profile_path, profile, target_property); /* Execute activation operation */
}
