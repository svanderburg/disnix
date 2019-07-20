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

#include "nixxml-glib.h"
#include "nixxml-generate-env-generic.h"
#include "nixxml-ghashtable.h"
#include "nixxml-gptrarray.h"

xmlChar **NixXML_generate_env_vars_with_generic_glib_data_structures(GHashTable *hash_table)
{
    NixXML_GenerateEnvGenericParams params;
    params.generate_list_env = NixXML_generate_env_value_from_g_ptrarray;
    return NixXML_generate_env_vars_from_g_hash_table(hash_table, &params, NixXML_generate_env_value_from_node);
}
