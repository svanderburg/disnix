/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2018  Sander van der Burg
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __DISNIX_XMLUTIL_H
#define __DISNIX_XMLUTIL_H

#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <glib.h>

/**
 * Creates an XML XPath object pointer from a XPath query on a
 * XML document.
 *
 * @param doc Pointer to XML document
 * @param xpath String containing the XPath query
 * @return XML XPath object pointer
 */
xmlXPathObjectPtr executeXPathQuery(xmlDocPtr doc, const char *xpath);

/**
 * Checks whether a given XML node has a text sub node and duplicates the text.
 *
 * @param node Pointer to XML node
 * @return Duplicated string contents, or NULL if the node is incorrect
 */
gchar *duplicate_node_text(xmlNodePtr node);

#endif
