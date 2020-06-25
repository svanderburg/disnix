/*
 * libnixxml - GLib integration with libnixxml
 * Copyright (C) 2019-2020  Sander van der Burg
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

#include "nixxml-ghashtable-iter.h"
#include <stdlib.h>

static int compare_keys(const void *l, const void *r)
{
    return g_strcmp0(*((gchar**)l), *((gchar**)r));
}

void NixXML_g_hash_table_ordered_iter_init(NixXML_GHashTableOrderedIter *iter, GHashTable *hash_table)
{
    iter->hash_table = hash_table;
    iter->keys = (gchar**)g_hash_table_get_keys_as_array(hash_table, &iter->length);
    iter->index = 0;
    qsort(iter->keys, iter->length, sizeof(gchar*), compare_keys);
}

gboolean NixXML_g_hash_table_ordered_iter_next(NixXML_GHashTableOrderedIter *iter, gchar **key, gpointer *value)
{
    if(iter->index < iter->length)
    {
        *key = iter->keys[iter->index];
        *value = g_hash_table_lookup(iter->hash_table, *key);
        iter->index++;

        return TRUE;
    }
    else
        return FALSE;
}

void NixXML_g_hash_table_ordered_iter_destroy(NixXML_GHashTableOrderedIter *iter)
{
    g_free(iter->keys);
}
