#ifndef __DISTRIBUTIONEXPORT_H
#define __DISTRIBUTIONEXPORT_H

#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <glib.h>
#include "distributionlist.h"

xmlXPathObjectPtr select_all_services_from_distribution(xmlDocPtr doc);

xmlXPathObjectPtr select_services_from_distribution(xmlDocPtr doc, gchar *service);

xmlXPathObjectPtr select_inter_dependencies(xmlDocPtr doc, gchar *service);

xmlXPathObjectPtr select_inter_dependend_services_from(xmlDocPtr doc, gchar *service);

xmlXPathObjectPtr select_all_distribution_items(xmlDocPtr doc);

int checkInterDependencies(xmlDocPtr doc);

xmlDocPtr create_distribution_export_doc(char *filename);

DistributionList *generate_distribution_list(xmlDocPtr doc);

#endif
