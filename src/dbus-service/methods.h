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

#ifndef __DISNIX_METHODS_H
#define __DISNIX_METHODS_H
#include "disnix-dbus.h"

gboolean on_handle_get_job_id(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation);

gboolean on_handle_import(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_closure);

gboolean on_handle_export(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *const *arg_derivation);

gboolean on_handle_print_invalid(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *const *arg_derivation);

gboolean on_handle_realise(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *const *arg_derivation);

gboolean on_handle_set(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_profile, const gchar *arg_derivation);

gboolean on_handle_query_installed(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_profile);

gboolean on_handle_query_requisites(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *const *arg_derivation);

gboolean on_handle_collect_garbage(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, gboolean arg_delete_old);

gboolean on_handle_activate(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_derivation, const gchar *arg_container, const gchar *arg_type, const gchar *const *arg_arguments);

gboolean on_handle_deactivate(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_derivation, const gchar *arg_container, const gchar *arg_type, const gchar *const *arg_arguments);

gboolean on_handle_lock(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_profile);

gboolean on_handle_unlock(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_profile);

gboolean on_handle_snapshot(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_derivation, const gchar *arg_container, const gchar *arg_type, const gchar *const *arg_arguments);

gboolean on_handle_restore(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_derivation, const gchar *arg_container, const gchar *arg_type, const gchar *const *arg_arguments);

gboolean on_handle_query_all_snapshots(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_container, const gchar *arg_component);

gboolean on_handle_query_latest_snapshot(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_container, const gchar *arg_component);

gboolean on_handle_print_missing_snapshots(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *const *arg_component);

gboolean on_handle_import_snapshots(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_container, const gchar *arg_component, const gchar *const *arg_snapshots);

gboolean on_handle_resolve_snapshots(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *const *arg_snapshots);

gboolean on_handle_clean_snapshots(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, gint arg_keep, const gchar *arg_container, const char *arg_component);

gboolean on_handle_delete_state(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid, const gchar *arg_derivation, const gchar *arg_container, const gchar *arg_type, const gchar *const *arg_arguments);

gboolean on_handle_get_logdir(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation);

gboolean on_handle_capture_config(OrgNixosDisnixDisnix *object, GDBusMethodInvocation *invocation, gint arg_pid);

#endif
