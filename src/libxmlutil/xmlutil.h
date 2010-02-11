#ifndef __XMLUTIL_H
#define __XMLUTIL_H

#include <libxml/parser.h>
#include <libxml/xpath.h>

xmlXPathObjectPtr executeXPathQuery(xmlDocPtr doc, char *xpath);

#endif
