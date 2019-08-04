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

#include "interfacestable.h"
#include <stdlib.h>
#include <nixxml-ghashtable.h>

GHashTable *parse_interfaces_table(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_g_hash_table_verbose(element, "interface", "name", userdata, parse_interface);
}

void delete_interfaces_table(GHashTable *interfaces_table)
{
    NixXML_delete_g_hash_table(interfaces_table, (NixXML_DeleteGHashTableValueFunc)delete_interface);
}

int check_interfaces_table(GHashTable *interfaces_table)
{
    return NixXML_check_g_hash_table(interfaces_table, (NixXML_CheckGHashTableValueFunc)check_interface);
}
