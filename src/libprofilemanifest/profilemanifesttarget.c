/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2019  Sander van der Burg
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
#include <nixxml-print-xml.h>

ProfileManifestTarget *parse_profile_manifest_target(gchar *profile, gchar *default_target)
{
    ProfileManifestTarget *profile_manifest_target = (ProfileManifestTarget*)g_malloc(sizeof(ProfileManifestTarget));
    gchar *manifest_file = g_strconcat(profile, "/profilemanifest.xml", NULL);
    profile_manifest_target->profile = profile;
    profile_manifest_target->profile_manifest = create_profile_manifest_from_file(manifest_file, default_target);
    g_free(manifest_file);
    return profile_manifest_target;
}

void delete_profile_manifest_target(ProfileManifestTarget *profile_manifest_target)
{
    if(profile_manifest_target != NULL)
    {
        g_free(profile_manifest_target->profile);
        delete_profile_manifest(profile_manifest_target->profile_manifest);
        g_free(profile_manifest_target);
    }
}

int check_profile_manifest_target(const ProfileManifestTarget *profile_manifest_target)
{
    if(profile_manifest_target->profile_manifest == NULL)
    {
        g_printerr("profilemanifesttarget.profileManifest is not set!\n");
        return FALSE;
    }
    else
        return check_profile_manifest(profile_manifest_target->profile_manifest);
}

static void print_profile_manifest_target_attributes_nix(FILE *file, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    ProfileManifestTarget *profile_manifest_target = (ProfileManifestTarget*)value;

    if(profile_manifest_target->profile != NULL)
        NixXML_print_attribute_nix(file, "profile", profile_manifest_target->profile, indent_level, userdata, NixXML_print_store_path_nix);
    NixXML_print_attribute_nix(file, "profileManifest", profile_manifest_target->profile_manifest, indent_level, userdata, (NixXML_PrintValueFunc)print_profile_manifest_nix);
}

void print_profile_manifest_target_nix(FILE *file, const ProfileManifestTarget *profile_manifest_target, const int indent_level, void *userdata)
{
    NixXML_print_attrset_nix(file, profile_manifest_target, indent_level, userdata, print_profile_manifest_target_attributes_nix, NULL);
}

static void print_profile_manifest_target_attributes_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    ProfileManifestTarget *profile_manifest_target = (ProfileManifestTarget*)value;

    if(profile_manifest_target->profile != NULL)
        NixXML_print_simple_attribute_xml(file, "profile", profile_manifest_target->profile, indent_level, type_property_name, userdata, NixXML_print_string_xml);
    NixXML_print_simple_attribute_xml(file, "profileManifest", profile_manifest_target->profile_manifest, indent_level, type_property_name, userdata, (NixXML_PrintXMLValueFunc)print_profile_manifest_xml);
}

void print_profile_manifest_target_xml(FILE *file, const ProfileManifestTarget *profile_manifest_target, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_simple_attrset_xml(file, profile_manifest_target, indent_level, type_property_name, userdata, print_profile_manifest_target_attributes_xml, NULL);
}
