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
#ifndef __DISNIX_CLEAN_SNAPSHOTS_H
#define __DISNIX_CLEAN_SNAPSHOTS_H
#include <glib.h>

/**
 * Iterates over targets defined in an infrastructure Nix expression and
 * performs the cleans all obsolete snapshots on each target.
 *
 * @param interface Path to the client interface executable
 * @param target_property Property in the infrastructure model which specifies
 *                        how to connect to the Disnix service
 * @param infrastructure_expr Path to the infrastructure expression
 * @param keep Number of snapshot generations to keep
 * @param container Name of the container to filter on, or NULL to consult all containers
 * @param component Name of the component to filter on, or NULL to consult all components
 * @param xml If set to TRUE it considers the input to be in XML format
 * @return 0 if everything succeeds, else a non-zero exit value
 */
int clean_snapshots(gchar *interface, gchar *target_property, gchar *infrastructure_expr, int keep, gchar *container, gchar *component, const int xml);

#endif
