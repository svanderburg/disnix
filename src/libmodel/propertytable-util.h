#ifndef __DISNIX_PROPERTYTABLE_UTIL_H
#define __DISNIX_PROPERTYTABLE_UTIL_H

#include "hashtable-util.h"

void delete_property_table(GHashTable *property_table);

int check_value_is_not_null(const gpointer value);

int check_property_table(GHashTable *property_table);

int compare_xml_strings(const xmlChar *left, const xmlChar *right);

int compare_property_tables(GHashTable *property_table1, GHashTable *property_table2);

#endif
