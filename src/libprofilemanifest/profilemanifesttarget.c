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

#include "profilemanifesttarget.h"
#include <nixxml-print-nix.h>
#include <nixxml-ghashtable.h>

void parse_profile_manifest_target(ProfileManifestTarget *profile_manifest_target)
{
    gchar *manifest_file = g_strconcat(profile_manifest_target->profile, "/manifest", NULL);
    profile_manifest_target->profile_manifest = create_profile_manifest_from_file(manifest_file);
    g_free(manifest_file);
}

void delete_profile_manifest_target_table(GHashTable *profile_manifest_target_table)
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, profile_manifest_target_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        ProfileManifestTarget *profile_manifest_target = (ProfileManifestTarget*)value;
        g_free(profile_manifest_target->profile);
        delete_profile_manifest(profile_manifest_target->profile_manifest);
        g_free(profile_manifest_target);
    }

    g_hash_table_destroy(profile_manifest_target_table);
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

static void print_profile_manifest_target_attributes_nix(FILE *file, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    ProfileManifestTarget *profile_manifest_target = (ProfileManifestTarget*)value;

    if(profile_manifest_target->profile != NULL)
        NixXML_print_attribute_nix(file, "profile", profile_manifest_target->profile, indent_level, userdata, NixXML_print_store_path_nix);
    NixXML_print_attribute_nix(file, "profileManifest", profile_manifest_target->profile_manifest, indent_level, userdata, print_profile_manifest_nix);
}

static void print_profile_manifest_target_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    NixXML_print_attrset_nix(file, value, indent_level, userdata, print_profile_manifest_target_attributes_nix, NULL);
}

void print_profile_manifest_target_table_nix(GHashTable *profile_manifest_target_table, void *userdata)
{
    NixXML_print_g_hash_table_nix(stdout, profile_manifest_target_table, 0, userdata, print_profile_manifest_target_nix);
}

static void print_profile_manifest_target_attributes_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    ProfileManifestTarget *profile_manifest_target = (ProfileManifestTarget*)value;

    if(profile_manifest_target->profile != NULL)
        NixXML_print_simple_attribute_xml(file, "profile", profile_manifest_target->profile, indent_level, type_property_name, userdata, NixXML_print_string_xml);
    NixXML_print_simple_attribute_xml(file, "profileManifest", profile_manifest_target->profile_manifest, indent_level, type_property_name, userdata, print_profile_manifest_xml);
}

static void print_profile_manifest_target_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_simple_attrset_xml(file, value, indent_level, type_property_name, userdata, print_profile_manifest_target_attributes_xml, NULL);
}

void print_profile_manifest_target_table_xml(GHashTable *profile_manifest_target_table, void *userdata)
{
    NixXML_print_open_root_tag(stdout, "profileManifestTargets");
    NixXML_print_g_hash_table_verbose_xml(stdout, profile_manifest_target_table, "target", "name", 0, NULL, userdata, print_profile_manifest_target_xml);
    NixXML_print_close_root_tag(stdout, "profileManifestTargets");
}

static int has_next_profile_manifest_target_process(void *data)
{
    ProfileManifestTargetIteratorData *iterator_data = (ProfileManifestTargetIteratorData*)data;
    return has_next_iteration_process(&iterator_data->model_iterator_data);
}

static pid_t next_profile_manifest_target_process(void *data)
{
    /* Declarations */
    ProfileManifestTargetIteratorData *iterator_data = (ProfileManifestTargetIteratorData*)data;

    /* Retrieve profile manifest target */
    void *key, *value;
    g_hash_table_iter_next(&iterator_data->iter, &key, &value);
    ProfileManifestTarget *profile_manifest_target = (ProfileManifestTarget*)value;

    /* Invoke the next profile manifest target item operation process */
    pid_t pid = iterator_data->map_profilemanifesttarget_item(iterator_data->data, (gchar*)key, profile_manifest_target);

    /* Increase the iterator index and update the pid table */
    next_iteration_process(&iterator_data->model_iterator_data, pid, profile_manifest_target);

    /* Return the pid of the invoked process */
    return pid;
}

static void complete_profile_manifest_target_process(void *data, pid_t pid, ProcReact_Status status, int result)
{
    ProfileManifestTargetIteratorData *iterator_data = (ProfileManifestTargetIteratorData*)data;

    /* Retrieve the completed item */
    ProfileManifestTarget *item = complete_iteration_process(&iterator_data->model_iterator_data, pid, status, result);

    /* Invoke callback that handles completion of distribution item */
    iterator_data->complete_profilemanifesttarget_item_mapping(iterator_data->data, item, status, result);
}

ProcReact_PidIterator create_profile_manifest_target_iterator(GHashTable *profile_manifest_target_table, map_profilemanifesttarget_item_function map_profilemanifesttarget_item, complete_profilemanifesttarget_item_mapping_function complete_profilemanifesttarget_item_mapping, void *data)
{
    ProfileManifestTargetIteratorData *profile_manifest_target_iterator_data = (ProfileManifestTargetIteratorData*)g_malloc(sizeof(ProfileManifestTargetIteratorData));

    init_model_iterator_data(&profile_manifest_target_iterator_data->model_iterator_data, g_hash_table_size(profile_manifest_target_table));
    profile_manifest_target_iterator_data->profile_manifest_target_table = profile_manifest_target_table;
    g_hash_table_iter_init(&profile_manifest_target_iterator_data->iter, profile_manifest_target_iterator_data->profile_manifest_target_table);
    profile_manifest_target_iterator_data->map_profilemanifesttarget_item = map_profilemanifesttarget_item;
    profile_manifest_target_iterator_data->complete_profilemanifesttarget_item_mapping = complete_profilemanifesttarget_item_mapping;
    profile_manifest_target_iterator_data->data = data;

    return procreact_initialize_pid_iterator(has_next_profile_manifest_target_process, next_profile_manifest_target_process, procreact_retrieve_boolean, complete_profile_manifest_target_process, profile_manifest_target_iterator_data);
}

void destroy_profile_manifest_target_iterator(ProcReact_PidIterator *iterator)
{
    ProfileManifestTargetIteratorData *iterator_data = (ProfileManifestTargetIteratorData*)iterator->data;
    destroy_model_iterator_data(&iterator_data->model_iterator_data);
    g_free(iterator_data);
}

int profile_manifest_target_iterator_has_succeeded(const ProcReact_PidIterator *iterator)
{
    ProfileManifestTargetIteratorData *iterator_data = (ProfileManifestTargetIteratorData*)iterator->data;
    return iterator_data->model_iterator_data.success;
}
