#ifndef __DISNIX_COMPAREUTIL_H
#define __DISNIX_COMPAREUTIL_H
#include <glib.h>

int compare_hash_tables(GHashTable *hash_table1, GHashTable *hash_table2, int (*compare_function) (const gpointer left, const gpointer right));

int compare_xml_strings(const gpointer left, const gpointer right);

int compare_property_tables(GHashTable *property_table1, GHashTable *property_table2);

#endif
