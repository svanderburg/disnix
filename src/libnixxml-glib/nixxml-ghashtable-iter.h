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

#ifndef __NIXXML_GHASHTABLE_ITER_H
#define __NIXXML_GHASHTABLE_ITER_H

#include <glib.h>

/**
 * @brief An iterator for a GHashTable that can be used to traverse the keys in a deterministic order.
 */
typedef struct
{
    GHashTable *hash_table;
    gchar **keys;
    guint length;
    guint index;
}
NixXML_GHashTableOrderedIter;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Has the same purpose as the g_hash_table_iter_init() function, but
 * works with an iterator that can be used to traverse a GHashTable in a
 * derministic order. When the work is done, it should be destroyed with the
 * NixXML_g_hash_table_ordered_iter_destroy() function.
 *
 * @param iter An uninitialized NixXML_GHashTableOrderedIter
 * @param hash_table A GHashTable
 */
void NixXML_g_hash_table_ordered_iter_init(NixXML_GHashTableOrderedIter *iter, GHashTable *hash_table);

/**
 * Has the same purpose as the g_hash_table_iter_next() function, but
 * works with an iterator that can be used to traverse a GHashTable in a
 * derministic order.
 *
 * @param iter A NixXML_GHashTableOrderedIter
 * @param key A location to store the key
 * @param value A location to store the value
 */
gboolean NixXML_g_hash_table_ordered_iter_next(NixXML_GHashTableOrderedIter *iter, gchar **key, gpointer *value);

/**
 * Destroys all the resources allocated with an intialized
 * NixXML_GHashTableOrderedIter struct.
 *
 * @param iter An initialized NixXML_GHashTableOrderedIter
 */
void NixXML_g_hash_table_ordered_iter_destroy(NixXML_GHashTableOrderedIter *iter);

#ifdef __cplusplus
}
#endif

#endif
