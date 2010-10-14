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
#include "collect-garbage.h"

static void print_usage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "disnix-collect-garbage [--interface interface] [--target-property targetProperty] [-d|--delete-old] infrastructure_expr\n");
    fprintf(stderr, "disnix-collect-garbage {-h | --help}\n");
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
	{"interface", required_argument, 0, 'i'},
	{"target-property", required_argument, 0, 't'},
	{"delete-old", no_argument, 0, 'd'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
    };
    char *interface = NULL;
    gboolean delete_old = FALSE;
    char *target_property = NULL;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "dh", long_options, &option_index)) != -1)
    {
	switch(c)
	{
	    case 'i':
		interface = optarg;
		break;
	    case 't':
		target_property = optarg;
		break;
	    case 'd':
	        delete_old = TRUE;
	        break;
	    case 'h':
		print_usage();
		return 0;
	}
    }
    
    /* Validate options */
    
    interface = check_interface_option(interface);
    target_property = check_target_property_option(target_property);
    
    if(optind >= argc)
    {
	fprintf(stderr, "An infrastructure Nix expression has to be specified!\n");
	return 1;
    }
    else
	return collect_garbage(interface, target_property, argv[optind], delete_old); /* Execute garbage collection operation */
}
