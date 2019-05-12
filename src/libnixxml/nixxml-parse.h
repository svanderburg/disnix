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

#ifndef __NIXXML_PARSE_H
#define __NIXXML_PARSE_H

#include <libxml/parser.h>

/**
 * Parses the content of a provided XML element.
 *
 * @param element XML element to parse.
 * @param userdata Arbitrary user data that is propagated to all parse functions
 */
typedef void *(*NixXML_ParseObjectFunc) (xmlNodePtr element, void *userdata);

/**
 * Creates an abstract data structure that serves as a container to store sub
 * elements.
 *
 * @param element XML element to parse.
 * @param userdata Arbitrary user data that is propagated to all parse functions
 */
typedef void *(*NixXML_CreateObjectFunc) (xmlNodePtr element, void *userdata);

/**
 * Adds a value to a list-like data structure.
 *
 * @param list A list-like data structure
 * @param value Value to add
 * @param userdata Arbitrary user data that is propagated to all parse functions
 */
typedef void (*NixXML_AddElementFunc) (void *list, void *value, void *userdata);

/**
 * Inserts a value into a table-like data structure.
 *
 * @param table A table-like data structure
 * @param key Attribute key
 * @param value Value to insert
 * @param userdata Arbitrary user data that is propagated to all parse functions
 */
typedef void (*NixXML_InsertObjectFunc) (void *table, const xmlChar *key, void *value, void *userdata);

/**
 * Finalizes a list-like data structure by discarding data it no longer needs.
 *
 * @param list A list-like data structure
 * @param userdata Arbitrary user data that is propagated to all parse functions
 */
typedef void *(*NixXML_FinalizeListFunc) (void *list, void *userdata);

/**
 * Parses an attribute set member and inserts it into a table-like data
 * structure in one go.
 *
 * @param element XML element to parse.
 * @param table A table-like data structure
 * @param key Attribute key
 * @param userdata Arbitrary user data that is propagated to all parse functions
 */
typedef void (*NixXML_ParseAndInsertObjectFunc) (xmlNodePtr element, void *table, const xmlChar *key, void *userdata);

/**
 * Searches for an element property with a provided name.
 *
 * @param element XML element to parse.
 * @param property_name Name of the property to search for
 * @return The property value or NULL if the property cannot be found
 */
xmlChar *NixXML_find_property(xmlNodePtr element, const char *property_name);

/**
 * Parses a value, e.g. a string, float, int or bool.
 *
 * @param element XML element to parse.
 * @param userdata Arbitrary user data that is propagated to all parse functions
 * @return A string representation of the value or NULL if it cannot be parsed. The caller needs to be free up the memory with xmlFree()
 */
void *NixXML_parse_value(xmlNodePtr element, void *userdata);

/**
 * Parses a list-like data structure.
 *
 * @param element XML element to parse.
 * @param child_element_name Name of the child XML elements in the list or NULL to accept any sub element as a list element.
 * @param userdata Arbitrary user data that is propagated to all parse functions
 * @param create_list Pointer to a function that constructs a list-like data structure
 * @param add_element Pointer to a function that adds an element to the list-like data structure
 * @param parse_object Pointer to a function that parses the value
 * @param finalize_list Pointer to a function that finalizes the list-like data structure by carrying out additional house keeping tasks
 * @return A list-like data structure
 */
void *NixXML_parse_list(xmlNodePtr element, const char *child_element_name, void *userdata, NixXML_CreateObjectFunc create_list, NixXML_AddElementFunc add_element, NixXML_ParseObjectFunc parse_object, NixXML_FinalizeListFunc finalize_list);

/**
 * Parses a heterogeneous table-like data structure using a simple XML notation
 * in which every sub element name represents a key and every content text the
 * value. Every element is parsed and inserted in one go. This function is
 * particularly useful when members of an attribute set could have different
 * types.
 *
 * @param element XML element to parse.
 * @param userdata Arbitrary user data that is propagated to all parse functions
 * @param create_table Pointer to a function that constructs a table-like data structure
 * @param parse_and_insert_object Pointer to a function that parses the value and inserts it into the the table-like data structure in one go
 * @return A table-like data structure
 */
void *NixXML_parse_simple_heterogeneous_attrset(xmlNodePtr element, void *userdata, NixXML_CreateObjectFunc create_table, NixXML_ParseAndInsertObjectFunc parse_and_insert_object);

/**
 * Parses a table-like data structure using a simple XML notation in which every
 * sub element name represents a key and every content text the value. Every
 * element is parsed according to a strategy and inserted into the table-like
 * data structure according to a strategy. This function is particularly useful
 * if all members of an attribute set have a uniform type.
 *
 * @param element XML element to parse.
 * @param userdata Arbitrary user data that is propagated to all parse functions
 * @param create_table Pointer to a function that constructs a table-like data structure
 * @param parse_object Pointer to a function that parses the value
 * @param insert_object Pointer to a function that inserts the value into the table-like data structure
 * @return A table-like data structure
 */
void *NixXML_parse_simple_attrset(xmlNodePtr element, void *userdata, NixXML_CreateObjectFunc create_table, NixXML_ParseObjectFunc parse_object, NixXML_InsertObjectFunc insert_object);

/**
 * Parses a table-like data structure using a simple XML notation in which every
 * sub element name represents a key and every content text the value. Every
 * element is parsed and inserted in one go. This function is particularly
 * useful when members of an attribute set could have different types.
 *
 * @param element XML element to parse.
 * @param child_element_name Name of the child XML elements in the attribute set or NULL to accept any sub element as an entry.
 * @param name_property_name Name of the attribute that refers to the name of attribute
 * @param userdata Arbitrary user data that is propagated to all parse functions
 * @param create_table Pointer to a function that constructs a table-like data structure
 * @param parse_and_insert_object Pointer to a function that parses the value and inserts it into the the table-like data structure in one go
 */
void *NixXML_parse_verbose_heterogeneous_attrset(xmlNodePtr element, const char *child_element_name, const char *name_property_name, void *userdata, NixXML_CreateObjectFunc create_table, NixXML_ParseAndInsertObjectFunc parse_and_insert_object);

/**
 * Parses a table-like data structure using a verbose XML notation in which every
 * sub element is annotated with a name attribute and the content text the value.
 * Every element is parsed according to a strategy and inserted into the
 * table-like data structure according to a strategy. This function is
 * particularly useful if all members of an attribute set have a uniform type.
 *
 * @param element XML element to parse.
 * @param child_element_name Name of the child XML elements in the attribute set or NULL to accept any sub element as an entry.
 * @param name_property_name Name of the attribute that refers to the name of attribute
 * @param userdata Arbitrary user data that is propagated to all parse functions
 * @param create_table Pointer to a function that constructs a table-like data structure
 * @param parse_object Pointer to a function that parses the value of the list element
 * @param insert_object Pointer to a function that inserts an object into the table-like data structure
 * @return A table-like data structure
 */
void *NixXML_parse_verbose_attrset(xmlNodePtr element, const char *child_element_name, const char *name_property_name, void *userdata, NixXML_CreateObjectFunc create_table, NixXML_ParseObjectFunc parse_object, NixXML_InsertObjectFunc insert_object);

#endif
