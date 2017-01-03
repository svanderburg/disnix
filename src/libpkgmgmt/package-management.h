/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2016  Sander van der Burg
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

#ifndef __DISNIX_PACKAGE_MANAGEMENT_H
#define __DISNIX_PACKAGE_MANAGEMENT_H

#include <glib.h>
#include <unistd.h>
#include <procreact_future.h>

pid_t pkgmgmt_import_closure(const char *closure, int stdout, int stderr);

gchar **pkgmgmt_export_closure(gchar *tmpdir, gchar **derivation, int stderr);

ProcReact_Future pkgmgmt_print_invalid_packages(gchar **derivation, int stderr);

ProcReact_Future pkgmgmt_realise(gchar **derivation, int stderr);

pid_t pkgmgmt_set_profile(gchar *profile, gchar *derivation, int stdout, int stderr);

ProcReact_Future pkgmgmt_query_requisites(gchar **derivation, int stderr);

pid_t pkgmgmt_collect_garbage(int delete_old, int stdout, int stderr);

ProcReact_Future pkgmgmt_instantiate(gchar *infrastructure_expr);

int pkgmgmt_set_coordinator_profile(const gchar *coordinator_profile_path, const gchar *manifest_file, const gchar *profile);

#endif
