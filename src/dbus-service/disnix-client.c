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

#include <stdio.h>
#include <getopt.h>
#include <glib.h>
#include <defaultoptions.h>
#include "operation.h"
#include "disnix-client-operation.h"

static void print_usage(const char *command)
{
    /* Print the usage */
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s [--session-bus] operation\n\n", command);
    
    fprintf(stderr, "Operations:\n");
    fprintf(stderr, "--import [--localfile|--remotefile] derivations\n");
    fprintf(stderr, "--export [--localfile|--remotefile] derivations\n");
    fprintf(stderr, "--print-invalid derivations\n");
    fprintf(stderr, "{-r|--realise} derivations\n");
    fprintf(stderr, "--set [{-p|--profile} name] derivation\n");
    fprintf(stderr, "{-q|--query-installed} [{-p|--profile} name]\n");
    fprintf(stderr, "--query-requisites derivations\n");
    fprintf(stderr, "--collect-garbage [{-d|--delete-old}]\n");
    fprintf(stderr, "--activate --type type --arguments arguments derivation\n");
    fprintf(stderr, "--deactivate --type type --arguments arguments derivation\n");
    fprintf(stderr, "--lock [{-p|--profile} profile]\n");
    fprintf(stderr, "--unlock [{-p|--profile} profile]\n");
    fprintf(stderr, "{-h|--help}\n");
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
	{"import", no_argument, 0, 'I'},
	{"export", no_argument, 0, 'E'},
	{"print-invalid", no_argument, 0, 'P'},
	{"realise", no_argument, 0, 'r'},
	{"set", no_argument, 0, 'S'},
	{"query-installed", no_argument, 0, 'q'},
	{"query-requisites", no_argument, 0, 'Q'},
	{"collect-garbage", no_argument, 0, 'C'},
	{"activate", no_argument, 0, 'A'},
	{"deactivate", no_argument, 0, 'D'},
	{"lock", no_argument, 0, 'L'},
	{"unlock", no_argument, 0, 'U'},
	{"help", no_argument, 0, 'h'},
	{"target", required_argument, 0, 't'},
	{"localfile", no_argument, 0, 'l'},
	{"remotefile", no_argument, 0, 'R'},
	{"profile", required_argument, 0, 'p'},
	{"delete-old", no_argument, 0, 'd'},
	{"type", required_argument, 0, 'T'},
	{"arguments", required_argument, 0, 'a'},
	{"session-bus", no_argument, 0, 'b'},
	{0, 0, 0, 0}
    };

    /* Option value declarations */
    Operation operation = OP_NONE;
    char *target, *profile = NULL, *type = NULL;
    gchar **derivation = NULL, **arguments = NULL;
    unsigned int derivation_size = 0, arguments_size = 0;
    int localfile = FALSE, remotefile = TRUE;
    int delete_old = FALSE, session_bus = FALSE;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "rqt:p:dh", long_options, &option_index)) != -1)
    {
	switch(c)
	{	    
	    case 'I':
		operation = OP_IMPORT;
		break;
	    case 'E':
		operation = OP_EXPORT;
		break;
	    case 'P':
		operation = OP_PRINT_INVALID;
		break;
	    case 'r':
		operation = OP_REALISE;
		break;
	    case 'S':
		operation = OP_SET;
		break;
	    case 'q':
		operation = OP_QUERY_INSTALLED;
		break;
	    case 'Q':
		operation = OP_QUERY_REQUISITES;
		break;
	    case 'C':
		operation = OP_COLLECT_GARBAGE;
		break;
	    case 'A':
		operation = OP_ACTIVATE;
		break;
	    case 'D':
		operation = OP_DEACTIVATE;
		break;
	    case 'L':
		operation = OP_LOCK;
		break;
	    case 'U':
		operation = OP_UNLOCK;
		break;
	    case 't':
		target = optarg;
		break;
	    case 'l':
		localfile = TRUE;
		remotefile = FALSE;
		break;
	    case 'R':
		localfile = FALSE;
		remotefile = TRUE;
		break;
	    case 'p':
		profile = optarg;
		break;
	    case 'd':
		delete_old = TRUE;
		break;
	    case 'T':
		type = optarg;
		break;
	    case 'a':
		arguments = (gchar**)g_realloc(arguments, (arguments_size + 1) * sizeof(gchar*));
		arguments[arguments_size] = g_strdup(optarg);
		arguments_size++;
		break;
	    case 'b':
		session_bus = TRUE;
		break;
	    case 'h':
	    case '?':
		print_usage(argv[0]);
		return 0;
	}
    }
    
    /* Validate options */
    profile = check_profile_option(profile);
    
    /* Validate non-options */
    while(optind < argc)
    {
	derivation = g_realloc(derivation, (derivation_size + 1) * sizeof(gchar*));
	derivation[derivation_size] = g_strdup(argv[optind]);
	derivation_size++;
	optind++;
    }
    
    derivation = g_realloc(derivation, (derivation_size + 1) * sizeof(gchar*));
    derivation[derivation_size] = NULL;
    
    /* Add NULL termination to the arguments vector */
    arguments = g_realloc(arguments, (arguments_size + 1) * sizeof(gchar*));
    arguments[arguments_size] = NULL;
    
    /* Execute Disnix client */
    return run_disnix_client(operation, derivation, session_bus, profile, delete_old, arguments, type);
}
