/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2022  Sander van der Burg
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

#include "profilemanifesttargettable.h"
#include <nixxml-print-nix.h>
#include <nixxml-ghashtable.h>

void delete_profile_manifest_target_table(GHashTable *profile_manifest_target_table)
{
    NixXML_delete_g_hash_table(profile_manifest_target_table, (NixXML_DeleteGHashTableValueFunc)delete_profile_manifest_target);
}

int check_profile_manifest_target_table(GHashTable *profile_manifest_target_table)
{
    return NixXML_check_g_hash_table(profile_manifest_target_table, (NixXML_CheckGHashTableValueFunc)check_profile_manifest_target);
}

void print_services_in_profile_manifest_target_table(GHashTable *profile_manifest_target_table)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, profile_manifest_target_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        ProfileManifestTarget *profile_manifest_target = (ProfileManifestTarget*)value;

        g_print("\nServices on: %s\n\n", (gchar*)key);
        print_services_in_profile_manifest(profile_manifest_target->profile_manifest);
    }
}

void print_services_per_container_in_profile_manifest_target_table(GHashTable *profile_manifest_target_table)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, profile_manifest_target_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        ProfileManifestTarget *profile_manifest_target = (ProfileManifestTarget*)value;

        g_print("\nServices on: %s\n\n", (gchar*)key);
        print_services_per_container_in_profile_manifest(profile_manifest_target->profile_manifest);
    }
}

void print_profile_manifest_target_table_nix(GHashTable *profile_manifest_target_table, void *userdata)
{
    NixXML_print_g_hash_table_nix(stdout, profile_manifest_target_table, 0, userdata, (NixXML_PrintValueFunc)print_profile_manifest_target_nix);
}

void print_profile_manifest_target_table_xml(GHashTable *profile_manifest_target_table, void *userdata)
{
    NixXML_print_open_root_tag(stdout, "profileManifestTargets");
    NixXML_print_g_hash_table_verbose_xml(stdout, profile_manifest_target_table, "target", "name", 0, NULL, userdata, (NixXML_PrintXMLValueFunc)print_profile_manifest_target_xml);
    NixXML_print_close_root_tag(stdout, "profileManifestTargets");
}
