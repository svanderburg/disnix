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
#include <nixxml-util.h>
#include "nixxml-ghashtable-iter.h"

void *NixXML_create_g_hash_table(xmlNodePtr element, void *userdata)
{
    return g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
}

void NixXML_delete_g_hash_table(GHashTable *hash_table, NixXML_DeleteGHashTableValueFunc delete_function)
{
    if(hash_table != NULL)
    {
        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init(&iter, hash_table);
        while(g_hash_table_iter_next(&iter, &key, &value))
            delete_function(value);

        g_hash_table_destroy(hash_table);
    }
}

void NixXML_delete_g_property_table(GHashTable *property_table)
{
    NixXML_delete_g_hash_table(property_table, (NixXML_DeleteGHashTableValueFunc)xmlFree);
}

int NixXML_check_g_hash_table(GHashTable *hash_table, NixXML_CheckGHashTableValueFunc check_function)
{
    GHashTableIter iter;
    gpointer key, value;
    int status = TRUE;

    g_hash_table_iter_init(&iter, hash_table);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        if(!check_function(value))
        {
            g_printerr("Hash table value with key: %s is invalid!\n", (gchar*)key);
            status = FALSE;
        }
    }

    return status;
}

int NixXML_check_g_property_table(GHashTable *property_table)
{
    return NixXML_check_g_hash_table(property_table, NixXML_check_value_is_not_null);
}

int NixXML_compare_g_hash_tables(GHashTable *hash_table1, GHashTable *hash_table2, NixXML_CompareGHashTableValueFunc compare_function)
{
    if(g_hash_table_size(hash_table1) == g_hash_table_size(hash_table2))
    {
        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init(&iter, hash_table1);
        while(g_hash_table_iter_next(&iter, &key, &value))
        {
            gpointer value2;

            if((value2 = g_hash_table_lookup(hash_table2, key)) == NULL)
                return FALSE;
            else
            {
                if(!compare_function(value, value2))
                    return FALSE;
            }
        }

        return TRUE;
    }
    else
        return FALSE;
}

int NixXML_compare_g_property_tables(GHashTable *property_table1, GHashTable *property_table2)
{
    return NixXML_compare_g_hash_tables(property_table1, property_table2, (NixXML_CompareGHashTableValueFunc)NixXML_compare_xml_strings);
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
    gpointer key, obj;

    g_hash_table_iter_init(&iter, hash_table);
    while(g_hash_table_iter_next(&iter, &key, &obj))
        NixXML_print_attribute_nix(file, (gchar*)key, obj, indent_level, userdata, print_value);
}

void NixXML_print_g_hash_table_nix(FILE *file, GHashTable *hash_table, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    NixXML_print_attrset_nix(file, hash_table, indent_level, userdata, NixXML_print_g_hash_table_attributes_nix, print_value);
}

void NixXML_print_g_hash_table_ordered_attributes_nix(FILE *file, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    GHashTable *hash_table = (GHashTable*)value;
    NixXML_GHashTableOrderedIter iter;
    gchar *key;
    gpointer obj;

    NixXML_g_hash_table_ordered_iter_init(&iter, hash_table);
    while(NixXML_g_hash_table_ordered_iter_next(&iter, &key, &obj))
        NixXML_print_attribute_nix(file, key, obj, indent_level, userdata, print_value);

    NixXML_g_hash_table_ordered_iter_destroy(&iter);
}

void NixXML_print_g_hash_table_ordered_nix(FILE *file, GHashTable *hash_table, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    NixXML_print_attrset_nix(file, hash_table, indent_level, userdata, NixXML_print_g_hash_table_ordered_attributes_nix, print_value);
}

/* Simple XML printing */

void NixXML_print_g_hash_table_simple_attributes_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    GHashTable *hash_table = (GHashTable*)value;
    GHashTableIter iter;
    gpointer key, obj;

    g_hash_table_iter_init(&iter, hash_table);
    while(g_hash_table_iter_next(&iter, &key, &obj))
        NixXML_print_simple_attribute_xml(file, (gchar*)key, obj, indent_level, type_property_name, userdata, print_value);
}

void NixXML_print_g_hash_table_simple_xml(FILE *file, GHashTable *hash_table, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    NixXML_print_simple_attrset_xml(file, hash_table, indent_level, type_property_name, userdata, NixXML_print_g_hash_table_simple_attributes_xml, print_value);
}

void NixXML_print_g_hash_table_simple_ordered_attributes_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    GHashTable *hash_table = (GHashTable*)value;
    NixXML_GHashTableOrderedIter iter;
    gchar *key;
    gpointer obj;

    NixXML_g_hash_table_ordered_iter_init(&iter, hash_table);
    while(NixXML_g_hash_table_ordered_iter_next(&iter, &key, &obj))
        NixXML_print_simple_attribute_xml(file, key, obj, indent_level, type_property_name, userdata, print_value);

    NixXML_g_hash_table_ordered_iter_destroy(&iter);
}

void NixXML_print_g_hash_table_simple_ordered_xml(FILE *file, GHashTable *hash_table, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    NixXML_print_simple_attrset_xml(file, hash_table, indent_level, type_property_name, userdata, NixXML_print_g_hash_table_simple_ordered_attributes_xml, print_value);
}

/* Verbose XML printing */

void NixXML_print_g_hash_table_verbose_attributes_xml(FILE *file, const void *value, const char *child_element_name, const char *name_property_name, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    GHashTable *hash_table = (GHashTable*)value;
    GHashTableIter iter;
    gpointer key, obj;

    g_hash_table_iter_init(&iter, hash_table);
    while(g_hash_table_iter_next(&iter, &key, &obj))
        NixXML_print_verbose_attribute_xml(file, child_element_name, name_property_name, (gchar*)key, obj, indent_level, type_property_name, userdata, print_value);
}

void NixXML_print_g_hash_table_verbose_xml(FILE *file, GHashTable *hash_table, const char *child_element_name, const char *name_property_name, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    NixXML_print_verbose_attrset_xml(file, hash_table, child_element_name, name_property_name, indent_level, type_property_name, userdata, NixXML_print_g_hash_table_verbose_attributes_xml, print_value);
}

void NixXML_print_g_hash_table_verbose_ordered_attributes_xml(FILE *file, const void *value, const char *child_element_name, const char *name_property_name, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    GHashTable *hash_table = (GHashTable*)value;
    NixXML_GHashTableOrderedIter iter;
    gchar *key;
    gpointer obj;

    NixXML_g_hash_table_ordered_iter_init(&iter, hash_table);
    while(NixXML_g_hash_table_ordered_iter_next(&iter, &key, &obj))
        NixXML_print_verbose_attribute_xml(file, child_element_name, name_property_name, key, obj, indent_level, type_property_name, userdata, print_value);

    NixXML_g_hash_table_ordered_iter_destroy(&iter);
}

void NixXML_print_g_hash_table_verbose_ordered_xml(FILE *file, GHashTable *hash_table, const char *child_element_name, const char *name_property_name, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    NixXML_print_verbose_attrset_xml(file, hash_table, child_element_name, name_property_name, indent_level, type_property_name, userdata, NixXML_print_g_hash_table_verbose_ordered_attributes_xml, print_value);
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

/* Generate environment variables functionality */

xmlChar **NixXML_generate_env_vars_from_g_hash_table(GHashTable *hash_table, void *userdata, NixXML_GenerateEnvValueFunc generate_value)
{
    GHashTableIter iter;
    gpointer key, obj;
    xmlChar **result = (xmlChar**)malloc((g_hash_table_size(hash_table) + 1) * sizeof(xmlChar*));
    unsigned int i = 0;

    g_hash_table_iter_init(&iter, hash_table);
    while(g_hash_table_iter_next(&iter, &key, &obj))
    {
        result[i] = NixXML_generate_env_variable((xmlChar*)key, obj, userdata, generate_value);
        i++;
    }

    result[i] = NULL; /* Add NULL termination to the end */
    return result;
}
