/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2011  Sander van der Burg
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
#define _GNU_SOURCE
#include <getopt.h>
#include "graph.h"

static void print_usage(char *command)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s manifest\n", command);
    fprintf(stderr, "%s {-h | --help}\n", command);
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
    };

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "h", long_options, &option_index)) != -1)
    {
	switch(c)
	{
	    case 'h':
		print_usage(argv[0]);
		return 0;
	}
    }

    if(optind >= argc)
    {
	fprintf(stderr, "ERROR: No manifest specified!\n");
	return 1;
    }
    else
    {
	/* Execute operation */
	return generate_graph(argv[optind]);
    }
}
