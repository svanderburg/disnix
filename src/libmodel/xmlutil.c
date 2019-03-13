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

#include "xmlutil.h"

gpointer parse_value(xmlNodePtr element)
{
    if(element->children != NULL && element->children->type == XML_TEXT_NODE)
        return g_strdup((gchar*)element->children->content);
    else
        return NULL;
}

GHashTable *parse_dictionary(xmlNodePtr element, ParseObjectFunc parse_object)
{
    xmlNodePtr element_children = element->children;
    GHashTable *table = g_hash_table_new(g_str_hash, g_str_equal);

    while(element_children != NULL)
    {
        gchar *key = g_strdup((gchar*)element_children->name);
        gpointer value = parse_object(element_children);
        g_hash_table_insert(table, key, value);
        element_children = element_children->next;
    }

    return table;
}

GPtrArray *parse_list(xmlNodePtr element, gchar *child_element_name, ParseObjectFunc parse_object)
{
    xmlNodePtr element_children = element->children;
    GPtrArray *return_array = g_ptr_array_new();

    while(element_children != NULL)
    {
        if(xmlStrcmp(element_children->name, (xmlChar*)child_element_name) == 0)
        {
            gpointer value = parse_object(element_children);
            g_ptr_array_add(return_array, value);
        }

        element_children = element_children->next;
    }

    return return_array;
}
