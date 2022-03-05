/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2022  Sander van der Burg
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

#ifndef __DISNIX_INTERFACESTABLE_H
#define __DISNIX_INTERFACESTABLE_H
#include <glib.h>
#include <libxml/parser.h>
#include <nixxml-types.h>
#include "interface.h"

/**
 * Parses an interfaces table from a sub section of an XML document.
 *
 * @param element Root XML element of the sub section that defines interfaces
 * @param userdata Arbitrary user data that is propagated to all parse functions
 * @return A pointer to an interfaces table
 */
GHashTable *parse_interfaces_table(xmlNodePtr element, void *userdata);

/**
 * Deletes an interfaces table from heap memory.
 *
 * @param interfaces_table Hash table with interfaces
 */
void delete_interfaces_table(GHashTable *interfaces_table);

/**
 * Checks whether an interace table is valid.
 *
 * @param interfaces_table Hash table with interfaces
 * @return TRUE if the interfaces all valid, else FALSE
 */
NixXML_bool check_interfaces_table(GHashTable *interfaces_table);

#endif
