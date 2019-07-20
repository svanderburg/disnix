/*
 * Copyright (c) 2019 Sander van der Burg
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so, 
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __NIXXML_GLIB_H
#define __NIXXML_GLIB_H
#include <libxml/parser.h>
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Generates a string vector of environment variable assignments from all
 * members in a given hash table of NixXML_Node values. It uses GPtrArrays
 * for representing lists and GHashTable for representing tables.
 *
 * @param hash_table Pointer to a GHashTable
 */
xmlChar **NixXML_generate_env_vars_with_generic_glib_data_structures(GHashTable *hash_table);

#ifdef __cplusplus
}
#endif

#endif
