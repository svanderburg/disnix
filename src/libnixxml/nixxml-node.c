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

#include "nixxml-node.h"
#include <libxml/parser.h>
#include <stdlib.h>

#define TRUE 1
#define FALSE 0

void NixXML_delete_node(NixXML_Node *node, NixXML_DeleteObjectFunc delete_list, NixXML_DeleteObjectFunc delete_table)
{
    if(node != NULL)
    {
        switch(node->type)
        {
            case NIX_XML_TYPE_STRING:
            case NIX_XML_TYPE_INT:
            case NIX_XML_TYPE_FLOAT:
            case NIX_XML_TYPE_BOOL:
                xmlFree(node->value);
                break;
            case NIX_XML_TYPE_LIST:
                delete_list(node->value);
                break;
            case NIX_XML_TYPE_ATTRSET:
                delete_table(node->value);
                break;
        }

        free(node);
    }
}

int NixXML_compare_nodes(const NixXML_Node *left, const NixXML_Node *right, NixXML_CompareObjectFunc compare_lists, NixXML_CompareObjectFunc compare_tables)
{
    if(left->type == right->type)
    {
        switch(left->type)
        {
            case NIX_XML_TYPE_STRING:
            case NIX_XML_TYPE_INT:
            case NIX_XML_TYPE_FLOAT:
            case NIX_XML_TYPE_BOOL:
                return (xmlStrcmp((xmlChar*)left->value, (xmlChar*)right->value) == 0);
            case NIX_XML_TYPE_LIST:
                return compare_lists(left->value, right->value);
            case NIX_XML_TYPE_ATTRSET:
                return compare_tables(left->value, right->value);
            default:
                return FALSE;
        }
    }
    else
        return FALSE;
}
