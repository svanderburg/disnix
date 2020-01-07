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

#include "nixxml-generate-env.h"

#define BUFFER_SIZE 50

xmlChar *NixXML_generate_env_value_from_string(const void *value, void *userdata)
{
    return xmlStrdup((const xmlChar*)value);
}

xmlChar *NixXML_generate_env_value_from_int(const void *value, void *userdata)
{
    xmlChar buffer[BUFFER_SIZE];
    sprintf((char*)buffer, "%d", *((const int*)value));
    return xmlStrdup(buffer);
}

xmlChar *NixXML_generate_env_value_from_float(const void *value, void *userdata)
{
    xmlChar buffer[BUFFER_SIZE];
    sprintf((char*)buffer, "%f", *((const float*)value));
    return xmlStrdup(buffer);
}

xmlChar *NixXML_generate_env_value_from_bool(const void *value, void *userdata)
{
    if(*((const int*)value))
        return xmlStrdup((xmlChar*) "1");
    else
        return xmlStrdup((xmlChar*) "");
}

xmlChar *NixXML_generate_env_value_from_bool_string(const void *value, void *userdata)
{
    if(xmlStrcmp((xmlChar*)value, (xmlChar*) "true") == 0)
        return xmlStrdup((xmlChar*) "1");
    else
        return xmlStrdup((xmlChar*) "");
}

xmlChar *NixXML_append_value_to_list_env_value(xmlChar *value, const xmlChar *add_value)
{
    if(value == NULL)
        return xmlStrdup(add_value);
    else
    {
        value = xmlStrcat(value, (xmlChar*) " ");
        value = xmlStrcat(value, add_value);
        return value;
    }
}

xmlChar *NixXML_generate_env_variable(const xmlChar *key, const void *value, void *userdata, NixXML_GenerateEnvValueFunc generate_value)
{
    xmlChar *result = xmlStrdup(key);
    xmlChar *env_value = generate_value(value, userdata);

    result = xmlStrcat(result, (xmlChar*) "=");
    result = xmlStrcat(result, env_value);

    xmlFree(env_value);
    return result;
}

void NixXML_delete_env_variable_array(xmlChar **array)
{
    if(array != NULL)
    {
        unsigned int i = 0;

        while(array[i] != NULL)
        {
            xmlFree(array[i]);
            i++;
        }

        free(array);
    }
}
