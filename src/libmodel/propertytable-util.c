#include "propertytable-util.h"

void delete_property_table(GHashTable *property_table)
{
    delete_hash_table(property_table, (DeleteFunction)xmlFree);
}

int check_value_is_not_null(const gpointer value)
{
    return (value != NULL);
}

int check_property_table(GHashTable *property_table)
{
    return check_hash_table(property_table, check_value_is_not_null);
}

int compare_xml_strings(const xmlChar *left, const xmlChar *right)
{
    return (xmlStrcmp(left, right) == 0);
}

int compare_property_tables(GHashTable *property_table1, GHashTable *property_table2)
{
    return compare_hash_tables(property_table1, property_table2, (CompareFunction)compare_xml_strings);
}
