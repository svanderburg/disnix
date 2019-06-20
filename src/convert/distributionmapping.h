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

#ifndef __DISNIX_DISTRIBUTIONMAPPING_H
#define __DISNIX_DISTRIBUTIONMAPPING_H
#include <glib.h>
#include <libxml/parser.h>
#include <targets.h>

/**
 * @brief Contains a mapping of a Nix profile to a disnix service target
 */
typedef struct
{
    /** Nix store path to the profile */
    xmlChar *profile;
    /** Address of a disnix service */
    xmlChar *target;
}
DistributionItem;

/**
 * Creates a new array with distribution items from the corresponding sub
 * section in an XML document.
 *
 * @param element XML root element of the sub section defining the mappings
 * @return GPtrArray with DistributionItems
 */
GPtrArray *parse_distribution(xmlNodePtr element);

/**
 * Deletes an array with distribution items.
 *
 * @param distribution_array Array with distribution items
 */
void delete_distribution_array(GPtrArray *distribution_array);

void print_distribution_array(const GPtrArray *distribution_array);

int check_distribution_array(const GPtrArray *distribution_array);

#endif
