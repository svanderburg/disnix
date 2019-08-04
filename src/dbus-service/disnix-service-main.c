/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2019  Sander van der Burg
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
#include "disnix-service.h"

#define TRUE 1
#define FALSE 0

static void print_usage(const char *command)
{
    printf("Usage: %s [OPTION]\n\n", command);

    puts(
    "The `disnix-service' tool is a daemon running on either the D-Bus system or\n"
    "session bus, which provides remote access to various deployment operations,\n"
    "such as importing, exporting, activating and deactivating services.\n\n"

    "The daemon is not very useful on its own, since it requires a wrapper that\n"
    "exposes the methods to remote users. The simplest wrapper that can be used is\n"
    "just running a SSH server and by using the `disnix-ssh-client' from the client\n"
    "machines.\n\n"

    "Other wrappers can also be used, which are basically thin layers that map the\n"
    "RPC protocol calls to D-Bus calls. A web service layer and client, for instance,\n"
    "is also available from the Disnix webpage which allows a user to use SOAP for\n"
    "executing deployment operations instead of SSH.\n\n"

    "Options:\n"
    "      --session-bus  Register the Disnix service on the session bus instead of\n"
    "                     the system bus (useful for testing)\n"
    "      --log-dir      Specify the directory in which the logfiles are stored\n"
    "                     (defaults to: /var/log/disnix)\n"
    "  -h, --help         Shows the usage of this command to the user\n"
    "  -v, --version      Shows the version of this command to the user\n"
    );
}

static void print_version(const char *command)
{
    printf(
    "%s (" PACKAGE_NAME ") " PACKAGE_VERSION "\n\n"
    "Copyright (C) 2008-2019 Sander van der Burg\n"
    , command);
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"session-bus", no_argument, 0, 's'},
        {"log-dir", required_argument, 0, 'l'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };
    
    int session_bus = FALSE;
    char *logdir = "/var/log/disnix";
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "hv", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case 's':
                session_bus = TRUE;
                break;
            case 'l':
                logdir = optarg;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            case '?':
                print_usage(argv[0]);
                return 1;
            case 'v':
                print_version(argv[0]);
                return 0;
        }
    }

    /* Start the program with the given options */
    return start_disnix_service(session_bus, logdir);
}
