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

#ifndef __NIXXML_PARSE_GENERIC_H
#define __NIXXML_PARSE_GENERIC_H

#include "nixxml-parse.h"
#include "nixxml-node.h"

/**
 * @brief Parameters propagated to all parsing functions that can be used to control to parsing process.
 */
typedef struct
{
    /** Name of the attribute that contains the type annotation */
    const char *type_property_name;
    /** Name of the attribute that refers to the name of attribute */
    const char *name_property_name;
    /** Pointer to a function that constructs a list-like data structure */
    NixXML_CreateObjectFunc create_list;
    /** Pointer to a function that constructs a table-like data structure */
    NixXML_CreateObjectFunc create_table;
    /** Pointer to a function that adds an element to the list-like data structure */
    NixXML_AddElementFunc add_element;
    /** Pointer to a function that inserts an object into the table-like data structure */
    NixXML_InsertObjectFunc insert_object;
    /** Pointer to a function that finalizes the list-like data structure by carrying out additional house keeping tasks */
    NixXML_FinalizeListFunc finalize_list;
}
NixXML_ParseExprParams;

/**
 * Parses a string node.
 *
 * @param element XML element to parse.
 * @param userdata Parameters propagated to all parse functions
 * @return A NiXML_Node with a string type
 */
void *NixXML_parse_generic_string(xmlNodePtr element, void *userdata);

/**
 * Parses an integer node.
 *
 * @param element XML element to parse.
 * @param userdata Parameters propagated to all parse functions
 * @return A NiXML_Node with an int type
 */
void *NixXML_parse_generic_int(xmlNodePtr element, void *userdata);

/**
 * Parses a float node.
 *
 * @param element XML element to parse.
 * @param userdata Parameters propagated to all parse functions
 * @return A NiXML_Node with a float type
 */
void *NixXML_parse_generic_float(xmlNodePtr element, void *userdata);

/**
 * Parser a boolean node.
 *
 * @param element XML element to parse.
 * @param userdata Parameters propagated to all parse functions
 * @return A NiXML_Node with a bool type
 */
void *NixXML_parse_generic_bool(xmlNodePtr element, void *userdata);

/**
 * Generically parses a list.
 *
 * @param element XML element to parse.
 * @param userdata Parameters propagated to all parse functions
 * @return A NiXML_Node with a bool type
 */
void *NixXML_create_generic_list(xmlNodePtr element, void *userdata);

/**
 * Generically parses an attribute set.
 *
 * @param element XML element to parse.
 * @param userdata Parameters propagated to all parse functions
 * @return A NiXML_Node with an attribute set type
 */
void *NixXML_create_generic_attrset(xmlNodePtr element, void *userdata);

/**
 * Adds a value to a list-like data structure.
 *
 * @param list A list-like data structure.
 * @param value An XML_Node object to add
 * @param userdata Parameters propagated to all parse functions
 */
void NixXML_add_element_to_generic_list(void *list, void *value, void *userdata);

/**
 * Inserts a value into a table-like data structure.
 *
 * @param table A table-like data structure.
 * @param key Attribute key
 * @param value An XML_Node object to insert
 * @param userdata Parameters propagated to all parse functions
 */
void NixXML_insert_object_into_generic_attrset(void *table, const xmlChar *key, void *value, void *userdata);

/**
 * Finalizes a generic list by executing house keeping tasks, such as cleaning
 * up obsolete resources.
 *
 * @param list A list-like data structure.
 * @param userdata Parameters propagated to all parse functions
 * @return The finalized list-like data structure.
 */
void *NixXML_finalize_generic_list(void *list, void *userdata);

/**
 * Generically parses an expression.
 *
 * @param element XML element to parse.
 * @param userdata Parameters propagated to all parse functions
 * @return A NixXML_Node
 */
void *NixXML_parse_expr(xmlNodePtr element, void *userdata);

/**
 * Recursively parses a type-annotated XML document.
 *
 * @param element Root element of the XML document
 * @param type_property_name Name of the attribute that contains the type annotation
 * @param name_property_name Name of the attribute that refers to the name of attribute
 * @param create_list Pointer to a function that constructs a list-like data structure
 * @param create_table Pointer to a function that constructs a table-like data structure
 * @param add_element Pointer to a function that adds an element to the list-like data structure
 * @param insert_object Pointer to a function that inserts an object into the table-like data structure
 * @param finalize_list Pointer to a function that finalizes the list-like data structure by carrying out additional house keeping tasks
 * @return A list-like or table-like data structure, string or NULL if the data cannot be parsed.
 */
NixXML_Node *NixXML_generic_parse_expr(xmlNodePtr element, const char *type_property_name, const char *name_property_name, NixXML_CreateObjectFunc create_list, NixXML_CreateObjectFunc create_table, NixXML_AddElementFunc add_element, NixXML_InsertObjectFunc insert_object, NixXML_FinalizeListFunc finalize_list);

#endif
