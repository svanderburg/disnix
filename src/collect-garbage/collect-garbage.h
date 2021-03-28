/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2021  Sander van der Burg
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
#ifndef __DISNIX_COLLECT_GARBAGE_H
#define __DISNIX_COLLECT_GARBAGE_H
#include <glib.h>

#define FLAG_COLLECT_GARBAGE_DELETE_OLD 0x1
#define FLAG_COLLECT_GARBAGE_XML        0x2

/**
 * Iterates over targets defined in an infrastructure Nix expression and
 * performs the garbage collection operation on each target.
 *
 * @param interface Path to the client interface executable
 * @param target_property Property in the infrastructure model which specifies
 *                        how to connect to the Disnix service
 * @param infrastructure_expr Path to the infrastructure expression
 * @param flags Configuration flags for the collect garbage operation
 * @return 0 if everything succeeds, else a non-zero exit value
 */
int collect_garbage(gchar *interface, gchar *target_property, gchar *infrastructure_expr, const unsigned int flags);

#endif
