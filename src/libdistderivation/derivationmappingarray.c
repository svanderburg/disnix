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
    if(derivation_mapping_array != NULL)
    {
        unsigned int i;

        for(i = 0; i < derivation_mapping_array->len; i++)
        {
            DerivationMapping *mapping = g_ptr_array_index(derivation_mapping_array, i);
            delete_derivation_mapping(mapping);
        }

        g_ptr_array_free(derivation_mapping_array, TRUE);
    }
}

int check_derivation_mapping_array(const GPtrArray *derivation_mapping_array)
{
    unsigned int i;

    for(i = 0; i < derivation_mapping_array->len; i++)
    {
        DerivationMapping *mapping = g_ptr_array_index(derivation_mapping_array, i);

        if(!check_derivation_mapping(mapping))
            return FALSE;
    }

    return TRUE;
}
