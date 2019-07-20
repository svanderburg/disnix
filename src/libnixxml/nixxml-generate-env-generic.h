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

#ifndef __NIXXML_GENERATE_ENV_GENERIC_H
#define __NIXXML_GENERATE_ENV_GENERIC_H

typedef struct NixXML_GenerateEnvGenericParams NixXML_GenerateEnvGenericParams;

#include <libxml/parser.h>
#include "nixxml-node.h"
#include "nixxml-generate-env.h"

typedef xmlChar *(*NixXML_GenerateListEnvValueFunc) (const void *value, void *userdata, NixXML_GenerateEnvValueFunc generate_value);

/**
 * Generic parameters for generating environment variables that get propagated to all generate functions
 */
struct NixXML_GenerateEnvGenericParams
{
    /** Pointer to a function that generates an environment variable representation of a list-like data structure */
    NixXML_GenerateListEnvValueFunc generate_list_env;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Converts an aribitary NixXML_Node to an environment variable representation.
 *
 * @param value Pointer to a data structure to convert
 * @param userdata Arbitrary user data that is propagated to all generate functions
 * @return The environment variable representation of the value
 */
xmlChar *NixXML_generate_env_value_from_node(const void *value, void *userdata);

/**
 * Converts an aribitary NixXML_Node to an environment variable representation.
 *
 * @param node Pointer to a NixXML_Node
 * @param generate_list_env Pointer to a function that generates an environment variable representation of a list-like data structure
 * @return The environment variable representation of the value
 */
xmlChar *NixXML_generate_env_generic(const NixXML_Node *node, NixXML_GenerateListEnvValueFunc generate_list_env);

#ifdef __cplusplus
}
#endif

#endif
