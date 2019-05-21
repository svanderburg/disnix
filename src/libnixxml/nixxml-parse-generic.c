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

#include "nixxml-parse-generic.h"

static void *parse_generic_value(xmlNodePtr element, NixXML_Type type, void *userdata)
{
    NixXML_Node *node = (NixXML_Node*)malloc(sizeof(NixXML_Node));
    node->type = type;
    node->value = NixXML_parse_value(element, userdata);
    return node;
}

void *NixXML_parse_generic_string(xmlNodePtr element, void *userdata)
{
    return parse_generic_value(element, NIX_XML_TYPE_STRING, userdata);
}

void *NixXML_parse_generic_int(xmlNodePtr element, void *userdata)
{
    return parse_generic_value(element, NIX_XML_TYPE_INT, userdata);
}

void *NixXML_parse_generic_float(xmlNodePtr element, void *userdata)
{
    return parse_generic_value(element, NIX_XML_TYPE_FLOAT, userdata);
}

void *NixXML_parse_generic_bool(xmlNodePtr element, void *userdata)
{
    return parse_generic_value(element, NIX_XML_TYPE_BOOL, userdata);
}

void *NixXML_create_generic_list(xmlNodePtr element, void *userdata)
{
    NixXML_ParseExprParams *params = (NixXML_ParseExprParams*)userdata;

    NixXML_Node *node = (NixXML_Node*)malloc(sizeof(NixXML_Node));
    node->type = NIX_XML_TYPE_LIST;
    node->value = params->create_list(element, userdata);
    return node;
}

void *NixXML_create_generic_attrset(xmlNodePtr element, void *userdata)
{
    NixXML_ParseExprParams *params = (NixXML_ParseExprParams*)userdata;

    NixXML_Node *node = (NixXML_Node*)malloc(sizeof(NixXML_Node));
    node->type = NIX_XML_TYPE_ATTRSET;
    node->value = params->create_table(element, userdata);
    return node;
}

void NixXML_add_element_to_generic_list(void *list, void *value, void *userdata)
{
    NixXML_ParseExprParams *params = (NixXML_ParseExprParams*)userdata;

    NixXML_Node *node = (NixXML_Node*)list;
    params->add_element(node->value, value, userdata);
}

void NixXML_insert_object_into_generic_attrset(void *table, const xmlChar *key, void *value, void *userdata)
{
    NixXML_ParseExprParams *params = (NixXML_ParseExprParams*)userdata;

    NixXML_Node *node = (NixXML_Node*)table;
    params->insert_object(node->value, key, value, userdata);
}

void *NixXML_finalize_generic_list(void *list, void *userdata)
{
    NixXML_ParseExprParams *params = (NixXML_ParseExprParams*)userdata;

    NixXML_Node *node = (NixXML_Node*)list;
    node->value = params->finalize_list(node->value, userdata);
    return node;
}

void *NixXML_parse_expr(xmlNodePtr element, void *userdata)
{
    NixXML_ParseExprParams *params = (NixXML_ParseExprParams*)userdata;

    xmlChar *type = NixXML_find_property(element, params->type_property_name);

    if(type == NULL)
    {
        fprintf(stderr, "Element: %s has no type annotation!\n", element->name);
        return NULL;
    }
    else if(xmlStrcmp(type, (xmlChar*) "list") == 0)
        return NixXML_parse_list(element, NULL, params, NixXML_create_generic_list, NixXML_add_element_to_generic_list, NixXML_parse_expr, NixXML_finalize_generic_list);
    else if(xmlStrcmp(type, (xmlChar*) "attrs") == 0)
        return NixXML_parse_verbose_attrset(element, NULL, params->name_property_name, params, NixXML_create_generic_attrset, NixXML_parse_expr, NixXML_insert_object_into_generic_attrset);
    else if(xmlStrcmp(type, (xmlChar*) "string") == 0)
        return NixXML_parse_generic_string(element, userdata);
    else if(xmlStrcmp(type, (xmlChar*) "int") == 0)
        return NixXML_parse_generic_int(element, userdata);
    else if(xmlStrcmp(type, (xmlChar*) "float") == 0)
        return NixXML_parse_generic_float(element, userdata);
    else if(xmlStrcmp(type, (xmlChar*) "bool") == 0)
        return NixXML_parse_generic_bool(element, userdata);
    else
    {
        fprintf(stderr, "Unknown type encountered: %s\n", type);
        return NULL;
    }
}

NixXML_Node *NixXML_generic_parse_expr(xmlNodePtr element, const char *type_property_name, const char *name_property_name, NixXML_CreateObjectFunc create_list, NixXML_CreateObjectFunc create_table, NixXML_AddElementFunc add_element, NixXML_InsertObjectFunc insert_object, NixXML_FinalizeListFunc finalize_list)
{
    NixXML_ParseExprParams params;
    params.type_property_name = type_property_name;
    params.name_property_name = name_property_name;
    params.create_list = create_list;
    params.create_table = create_table;
    params.add_element = add_element;
    params.insert_object = insert_object;
    params.finalize_list = finalize_list;
    return (NixXML_Node*)NixXML_parse_expr(element, &params);
}
