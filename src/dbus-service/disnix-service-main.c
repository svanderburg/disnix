/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2015  Sander van der Burg
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
#include "disnix-service-main.h"

#define TRUE 1
#define FALSE 0

static void print_usage(const char *command)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s [options]\n\n", command);
    
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "--dysnomia-modules-dir    Directory where the Dysnomia modules can be found\n");
    fprintf(stderr, "--session-bus             Register the Disnix service on the session bus instead of the system bus (useful for testing)\n");
    fprintf(stderr, "--help                    Shows the usage of this command to the user\n");
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
	{"session-bus", no_argument, 0, 's'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
    };
    
    int session_bus = FALSE;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "h", long_options, &option_index)) != -1)
    {
	switch(c)
	{
	    case 's':
		session_bus = TRUE;
		break;
	    case 'h':
	    case '?':
		print_usage(argv[0]);
		return 0;
	}
    }

    /* Start the program with the given options */
    return start_disnix_service(session_bus);
}
