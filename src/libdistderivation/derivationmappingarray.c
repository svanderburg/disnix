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

#include "derivationmappingarray.h"
#include <nixxml-gptrarray.h>

GPtrArray *parse_derivation_mapping_array(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_g_ptr_array(element, "mapping", userdata, parse_derivation_mapping);
}

void delete_derivation_mapping_array(GPtrArray *derivation_mapping_array)
{
    NixXML_delete_g_ptr_array(derivation_mapping_array, (NixXML_DeleteGPtrArrayElementFunc)delete_derivation_mapping);
}

int check_derivation_mapping_array(const GPtrArray *derivation_mapping_array)
{
    return NixXML_check_g_ptr_array(derivation_mapping_array, (NixXML_CheckGPtrArrayElementFunc)check_derivation_mapping);
}
