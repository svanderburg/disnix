/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2018  Sander van der Burg
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

#ifndef __DISNIX_DERIVATIONMAPPINGARRAY_H
#define __DISNIX_DERIVATIONMAPPINGARRAY_H
#include <libxml/parser.h>
#include <glib.h>
#include "interfacestable.h"
#include "derivationmapping.h"

/**
 * Parses all derivation mappings in a sub section of an XML document.
 *
 * @param element Root XML element of the sub section that defines derivation items
 * @return GPtrArray of derivation items.
 */
GPtrArray *parse_derivation_mapping_array(xmlNodePtr element, void *userdata);

/**
 * Deletes an array with derivation items
 *
 * @param derivation_array Derivation array to delete
 */
void delete_derivation_mapping_array(GPtrArray *derivation_mapping_array);

int check_derivation_mapping_array(const GPtrArray *derivation_mapping_array);

#endif
