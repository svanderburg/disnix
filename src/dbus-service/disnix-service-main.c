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

#define _GNU_SOURCE
#include <stdio.h>
#include <getopt.h>
#define TRUE 1
#define FALSE 0

static void print_usage()
{
    printf("Usage:\n");
    printf("disnix-service [options]\n\n");
    
    printf("Options:\n");
    printf("--activation-modules-dir  Directory where the activation modules can be found\n");
    printf("--lock-manager            Optional process which is invoked when executing the lock method\n");
    printf("--unlock-manager          Optional process which is invoked when executing the unlock method\n");
    printf("--session-bus             Register the Disnix service on the session bus instead of the system bus (useful for testing)\n");
    printf("--help                    Shows the usage of this command to the user\n");
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
	{"activation-modules-dir", required_argument, 0, 'a'},
	{"lock-manager", required_argument, 0, 'l'},
	{"unlock-manager", required_argument, 0, 'u'}, 
	{"session-bus", no_argument, 0, 's'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
    };
    
    char *activation_modules_dir = NULL, *lock_manager = NULL, *unlock_manager = NULL;
    int session_bus = FALSE;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "h", long_options, &option_index)) != -1)
    {
	switch(c)
	{
	    case 'a':
		activation_modules_dir = optarg;
		break;
	    case 'l':
	        lock_manager = optarg;
		break;
	    case 'u':
	        unlock_manager = optarg;
		break;
	    case 's':
		session_bus = TRUE;
		break;
	    case 'h':
		print_usage();
		return 0;
	}
    }

    /* Start the program with the given options */
    return start_disnix_service(activation_modules_dir, lock_manager, unlock_manager, session_bus);
}
