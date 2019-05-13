/*
 * libnixxml - GLib integration with libnixxml
 * Copyright (C) 2019  Sander van der Burg
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

#include "nixxml-ghashtable.h"

void *NixXML_create_g_hash_table(xmlNodePtr element, void *userdata)
{
    return g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
}

void NixXML_insert_into_g_hash_table(void *table, const xmlChar *key, void *value, void *userdata)
{
    GHashTable *hash_table = (GHashTable*)table;
    g_hash_table_insert(hash_table, g_strdup((gchar*)key), value);
}

/* Nix printing */

void NixXML_print_g_hash_table_attributes_nix(FILE *file, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    GHashTable *hash_table = (GHashTable*)value;
    GHashTableIter iter;
    gpointer *key;
    gpointer *obj;

    g_hash_table_iter_init(&iter, hash_table);
    while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&obj))
        NixXML_print_attribute_nix(file, (gchar*)key, obj, indent_level, userdata, print_value);
}

void NixXML_print_g_hash_table_nix(FILE *file, GHashTable *hash_table, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    NixXML_print_attrset_nix(file, hash_table, indent_level, userdata, NixXML_print_g_hash_table_attributes_nix, print_value);
}

/* Simple XML printing */

void NixXML_print_g_hash_table_simple_attributes_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    GHashTable *hash_table = (GHashTable*)value;
    GHashTableIter iter;
    gpointer *key;
    gpointer *obj;

    g_hash_table_iter_init(&iter, hash_table);
    while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&obj))
        NixXML_print_simple_attribute_xml(file, (gchar*)key, obj, indent_level, type_property_name, userdata, print_value);
}

void NixXML_print_g_hash_table_simple_xml(FILE *file, GHashTable *hash_table, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    NixXML_print_simple_attrset_xml(file, hash_table, indent_level, type_property_name, userdata, NixXML_print_g_hash_table_simple_attributes_xml, print_value);
}

/* Verbose XML printing */

void NixXML_print_g_hash_table_verbose_attributes_xml(FILE *file, const void *value, const char *child_element_name, const char *name_property_name, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    GHashTable *hash_table = (GHashTable*)value;
    GHashTableIter iter;
    gpointer *key;
    gpointer *obj;

    g_hash_table_iter_init(&iter, hash_table);
    while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&obj))
        NixXML_print_verbose_attribute_xml(file, child_element_name, name_property_name, (gchar*)key, obj, indent_level, type_property_name, userdata, print_value);
}

void NixXML_print_g_hash_table_verbose_xml(FILE *file, GHashTable *hash_table, const char *child_element_name, const char *name_property_name, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    NixXML_print_verbose_attrset_xml(file, hash_table, child_element_name, name_property_name, indent_level, type_property_name, userdata, NixXML_print_g_hash_table_verbose_attributes_xml, print_value);
}

/* Parse functionality */

void *NixXML_parse_g_hash_table_simple(xmlNodePtr element, void *userdata, NixXML_ParseObjectFunc parse_object)
{
    return NixXML_parse_simple_attrset(element, userdata, NixXML_create_g_hash_table, parse_object, NixXML_insert_into_g_hash_table);
}

void *NixXML_parse_g_hash_table_verbose(xmlNodePtr element, const char *child_element_name, const char *name_property_name, void *userdata, NixXML_ParseObjectFunc parse_object)
{
    return NixXML_parse_verbose_attrset(element, child_element_name, name_property_name, userdata, NixXML_create_g_hash_table, parse_object, NixXML_insert_into_g_hash_table);
}
