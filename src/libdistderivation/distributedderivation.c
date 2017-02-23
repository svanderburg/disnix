/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2017  Sander van der Burg
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

#include "distributedderivation.h"
#include "derivationmapping.h"
#include "interfaces.h"

DistributedDerivation *create_distributed_derivation(const gchar *distributed_derivation_file)
{
    GPtrArray *derivation_array, *interface_array;
    DistributedDerivation *distributed_derivation;
    
    derivation_array = create_derivation_array(distributed_derivation_file);
    if(derivation_array == NULL)
        return NULL;
    
    interface_array = create_interface_array(distributed_derivation_file);
    if(interface_array == NULL)
    {
        delete_derivation_array(derivation_array);
        return NULL;
    }
    
    distributed_derivation = (DistributedDerivation*)g_malloc(sizeof(DistributedDerivation));
    distributed_derivation->derivation_array = derivation_array;
    distributed_derivation->interface_array = interface_array;
    
    return distributed_derivation;
}

void delete_distributed_derivation(DistributedDerivation *distributed_derivation)
{
    if(distributed_derivation != NULL)
    {
        delete_derivation_array(distributed_derivation->derivation_array);
        delete_interface_array(distributed_derivation->interface_array);
        g_free(distributed_derivation);
    }
}
