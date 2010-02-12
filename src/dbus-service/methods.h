/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2010  Sander van der Burg
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

#ifndef __METHODS_H
#define __METHODS_H
#include <glib.h>
#include "disnix-instance.h"

gboolean disnix_acknowledge(DisnixObject *object, gchar *pid, GError **error);

gboolean disnix_import(DisnixObject *object, gchar *closure, gchar **pid, GError **error);

gboolean disnix_export(DisnixObject *object, gchar **derivation, gchar **pid, GError **error);

gboolean disnix_print_invalid(DisnixObject *object, gchar **derivation, gchar **pid, GError **error);

gboolean disnix_realise(DisnixObject *object, gchar **derivation, gchar **pid, GError **error);

gboolean disnix_set(DisnixObject *object, const gchar *profile, const gchar *derivation, gchar **pid, GError **error);

gboolean disnix_query_installed(DisnixObject *object, const gchar *profile, gchar **pid, GError **error);

gboolean disnix_query_requisites(DisnixObject *object, gchar **derivation, gchar **pid, GError **error);

gboolean disnix_collect_garbage(DisnixObject *object, const gboolean delete_old, gchar **pid, GError **error);

gboolean disnix_activate(DisnixObject *object, const gchar *derivation, const gchar *type, gchar **arguments, gchar **pid, GError **error);

gboolean disnix_deactivate(DisnixObject *object, const gchar *derivation, const gchar *type, gchar **arguments, gchar **pid, GError **error);

gboolean disnix_lock(DisnixObject *object, gchar **pid, GError **error);

gboolean disnix_unlock(DisnixObject *object, gchar **pid, GError **error);

#endif
