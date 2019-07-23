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

#ifndef __DISNIX_DERIVATIONMAPPING_H
#define __DISNIX_DERIVATIONMAPPING_H
#include <libxml/parser.h>
#include <glib.h>

/**
 * @brief Contains a mapping of a Nix store derivation to a Disnix Service target
 */
typedef struct
{
    /** Nix store derivation path */
    xmlChar *derivation;
    /** Address of a disnix service */
    xmlChar *interface;
    /** Nix store paths of the build result, or NULL if it has not yet been realised */
    gchar **result;
}
DerivationMapping;

void *parse_derivation_mapping(xmlNodePtr element, void *userdata);

void delete_derivation_mapping(DerivationMapping *mapping);

int check_derivation_mapping(const DerivationMapping *mapping);

#endif
