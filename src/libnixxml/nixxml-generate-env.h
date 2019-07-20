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

#ifndef __NIXXML_GENERATE_ENV_H
#define __NIXXML_GENERATE_ENV_H

#include <libxml/parser.h>

/**
 * Generates an environment variable representation of a value.
 *
 * @param value Pointer to a data structure to convert
 * @param userdata Arbitrary user data that is propagated to all generate functions
 * @return The environment variable representation of the value
 */
typedef xmlChar *(*NixXML_GenerateEnvValueFunc) (const void *value, void *userdata);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Converts a string value to an environment variable representation.
 *
 * @param value Pointer to a data structure to convert
 * @param userdata Arbitrary user data that is propagated to all generate functions
 * @return The environment variable representation of the value
 */
xmlChar *NixXML_generate_env_value_from_string(const void *value, void *userdata);

/**
 * Converts an integer value to an environment variable representation.
 *
 * @param value Pointer to a data structure to convert
 * @param userdata Arbitrary user data that is propagated to all generate functions
 * @return The environment variable representation of the value
 */
xmlChar *NixXML_generate_env_value_from_int(const void *value, void *userdata);

/**
 * Converts a float value to an environment variable representation.
 *
 * @param value Pointer to a data structure to convert
 * @param userdata Arbitrary user data that is propagated to all generate functions
 * @return The environment variable representation of the value
 */
xmlChar *NixXML_generate_env_value_from_float(const void *value, void *userdata);

/**
 * Converts a boolean value to an environment variable representation.
 *
 * @param value Pointer to a data structure to convert
 * @param userdata Arbitrary user data that is propagated to all generate functions
 * @return The environment variable representation of the value
 */
xmlChar *NixXML_generate_env_value_from_bool(const void *value, void *userdata);

/**
 * Appends another string value to a generate environment for a list.
 *
 * @param value A partial environment variable representation of a list
 * @param add_value Value to add
 * @return A whitespace separated string with the value appended
 */
xmlChar *NixXML_append_value_to_list_env_value(xmlChar *value, const xmlChar *add_value);

/**
 * Generates a string containing an environment variable assignment: key=value
 *
 * @param key Name of the environment variable
 * @param value Value of the environment variable
 * @param userdata Arbitrary user data that is propagated to all generate functions
 * @param generate_value A pointer to a function that generates an environment variable representation of a provided value
 */
xmlChar *NixXML_generate_env_variable(const xmlChar *key, const void *value, void *userdata, NixXML_GenerateEnvValueFunc generate_value);

/**
 * Deletes a generated environment variable vector from heap memory.
 *
 * @param array Environment variable to deallocate
 */
void NixXML_delete_env_variable_array(xmlChar **array);

#ifdef __cplusplus
}
#endif

#endif
