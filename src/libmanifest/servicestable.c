#include "servicestable.h"
#include <nixxml-ghashtable.h>
#include "hashtable-util.h"

GHashTable *parse_services_table(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_g_hash_table_verbose(element, "service", "name", userdata, parse_manifest_service);
}

void delete_services_table(GHashTable *services_table)
{
    delete_hash_table(services_table, (DeleteFunction)delete_manifest_service);
}

int check_services_table(GHashTable *services_table)
{
    if(services_table == NULL)
        return TRUE;
    else
        return check_hash_table(services_table, (CheckFunction)check_manifest_service);
}

int compare_services_tables(GHashTable *services_table1, GHashTable *services_table2)
{
    return compare_hash_tables(services_table1, services_table2, (CompareFunction)compare_manifest_services);
}

GHashTable *generate_union_services_table(GHashTable *left, GHashTable *right)
{
    GHashTable *result_table = g_hash_table_new(g_str_hash, g_str_equal);
    GHashTableIter iter;
    gpointer key, value;

    /* Insert all elements from left */
    g_hash_table_iter_init(&iter, left);
    while(g_hash_table_iter_next(&iter, &key, &value))
        g_hash_table_insert(result_table, key, value);

    /* Insert all elements from right that have not been inserted yet */
    g_hash_table_iter_init(&iter, right);
    while(g_hash_table_iter_next(&iter, &key, &value))
    {
        if(!g_hash_table_contains(left, key))
            g_hash_table_insert(result_table, key, value);
    }

    return result_table;
}

void print_services_table_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    NixXML_print_g_hash_table_ordered_nix(file, (GHashTable*)value, indent_level, userdata, (NixXML_PrintValueFunc)print_manifest_service_nix);
}

void print_services_table_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_g_hash_table_verbose_ordered_xml(file, (GHashTable*)value, "service", "name", indent_level, NULL, userdata, (NixXML_PrintXMLValueFunc)print_manifest_service_xml);
}
