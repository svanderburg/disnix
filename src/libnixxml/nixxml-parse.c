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

#include "nixxml-parse.h"
#include <stdio.h>
#include <stdlib.h>

xmlChar *NixXML_find_property(xmlNodePtr element, const char *property_name)
{
    xmlAttrPtr element_property = element->properties;
    xmlChar *result = NULL;

    while(element_property != NULL)
    {
        if(xmlStrcmp(element_property->name, (const xmlChar*) property_name) == 0)
            result = element_property->children->content;

        element_property = element_property->next;
    }

    return result;
}

void *NixXML_parse_value(xmlNodePtr element, void *userdata)
{
    if(element->children != NULL && element->children->type == XML_TEXT_NODE)
        return xmlStrdup(element->children->content);
    else
        return NULL;
}

void *NixXML_parse_int(xmlNodePtr element, void *userdata)
{
    if(element->children != NULL && element->children->type == XML_TEXT_NODE)
    {
        int *int_value = (int*)malloc(sizeof(int));
        *int_value = atoi((char*)element->children->content);
        return int_value;
    }
    else
        return NULL;
}

void *NixXML_parse_list(xmlNodePtr element, const char *child_element_name, void *userdata, NixXML_CreateObjectFunc create_list, NixXML_AddElementFunc add_element, NixXML_ParseObjectFunc parse_object, NixXML_FinalizeListFunc finalize_list)
{
    void *list = create_list(element, userdata);
    xmlNodePtr child_element = element->children;

    while(child_element != NULL)
    {
        if(child_element->type == XML_ELEMENT_NODE && (child_element_name == NULL || xmlStrcmp(child_element->name, (const xmlChar*)child_element_name) == 0))
        {
            void *value = parse_object(child_element, userdata);
            add_element(list, value, userdata);
        }

        child_element = child_element->next;
    }

    return finalize_list(list, userdata);
}

typedef void (*ProcessValueFunc) (xmlNodePtr child_element, void *table, const xmlChar *key, void *paramsdata, void *userdata);

typedef struct
{
    NixXML_ParseAndInsertObjectFunc parse_and_insert_object;
}
ParseAndInsertSingleFunctionParam;

static void execute_parse_and_insert(xmlNodePtr child_element, void *table, const xmlChar *key, void *paramsdata, void *userdata)
{
    ParseAndInsertSingleFunctionParam *params = (ParseAndInsertSingleFunctionParam*)paramsdata;
    params->parse_and_insert_object(child_element, table, key, userdata);
}

typedef struct
{
    NixXML_ParseObjectFunc parse_object;
    NixXML_InsertObjectFunc insert_object;
}
ParseAndInsertFunctionsParams;

static void execute_parse_and_insert_functions(xmlNodePtr child_element, void *table, const xmlChar *key, void *paramsdata, void *userdata)
{
    ParseAndInsertFunctionsParams *params = (ParseAndInsertFunctionsParams*)paramsdata;
    void *value = params->parse_object(child_element, userdata);
    params->insert_object(table, key, value, userdata);
}

static void *parse_simple_attrset(xmlNodePtr element, NixXML_CreateObjectFunc create_table, ProcessValueFunc process_value, void *paramsdata, void *userdata)
{
    void *table = create_table(element, userdata);
    xmlNodePtr child_element = element->children;

    while(child_element != NULL)
    {
        if(child_element->type == XML_ELEMENT_NODE)
            process_value(child_element, table, child_element->name, paramsdata, userdata);

        child_element = child_element->next;
    }

    return table;
}

void *NixXML_parse_simple_heterogeneous_attrset(xmlNodePtr element, void *userdata, NixXML_CreateObjectFunc create_table, NixXML_ParseAndInsertObjectFunc parse_and_insert_object)
{
    ParseAndInsertSingleFunctionParam params;
    params.parse_and_insert_object = parse_and_insert_object;
    return parse_simple_attrset(element, create_table, execute_parse_and_insert, &params, userdata);
}

void *NixXML_parse_simple_attrset(xmlNodePtr element, void *userdata, NixXML_CreateObjectFunc create_table, NixXML_ParseObjectFunc parse_object, NixXML_InsertObjectFunc insert_object)
{
    ParseAndInsertFunctionsParams params;
    params.parse_object = parse_object;
    params.insert_object = insert_object;
    return parse_simple_attrset(element, create_table, execute_parse_and_insert_functions, &params, userdata);
}

static void *parse_verbose_attrset(xmlNodePtr element, const char *child_element_name, const char *name_property_name, NixXML_CreateObjectFunc create_table, ProcessValueFunc process_value, void *paramsdata, void *userdata)
{
    void *table = create_table(element, userdata);
    xmlNodePtr child_element = element->children;

    while(child_element != NULL)
    {
        if(child_element->type == XML_ELEMENT_NODE && (child_element_name == NULL || xmlStrcmp(child_element->name, (const xmlChar*)child_element_name) == 0))
        {
            xmlChar *key = NixXML_find_property(child_element, name_property_name);

            if(key != NULL)
                process_value(child_element, table, key, paramsdata, userdata);
        }

        child_element = child_element->next;
    }

    return table;
}

void *NixXML_parse_verbose_heterogeneous_attrset(xmlNodePtr element, const char *child_element_name, const char *name_property_name, void *userdata, NixXML_CreateObjectFunc create_table, NixXML_ParseAndInsertObjectFunc parse_and_insert_object)
{
    ParseAndInsertSingleFunctionParam params;
    params.parse_and_insert_object = parse_and_insert_object;
    return parse_verbose_attrset(element, child_element_name, name_property_name, create_table, execute_parse_and_insert, &params, userdata);
}

void *NixXML_parse_verbose_attrset(xmlNodePtr element, const char *child_element_name, const char *name_property_name, void *userdata, NixXML_CreateObjectFunc create_table, NixXML_ParseObjectFunc parse_object, NixXML_InsertObjectFunc insert_object)
{
    ParseAndInsertFunctionsParams params;
    params.parse_object = parse_object;
    params.insert_object = insert_object;
    return parse_verbose_attrset(element, child_element_name, name_property_name, create_table, execute_parse_and_insert_functions, &params, userdata);
}
