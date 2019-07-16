#include "compareutil.h"
#include <libxml/parser.h>

int compare_hash_tables(GHashTable *hash_table1, GHashTable *hash_table2, int (*compare_function) (const gpointer left, const gpointer right))
{
    if(g_hash_table_size(hash_table1) == g_hash_table_size(hash_table2))
    {
        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init(&iter, hash_table1);
        while(g_hash_table_iter_next(&iter, &key, &value))
        {
            gpointer value2;

            if((value2 = g_hash_table_lookup(hash_table2, key)) == NULL)
                return FALSE;
            else
            {
                if(!compare_function(value, value2))
                    return FALSE;
            }
        }

        return TRUE;
    }
    else
        return FALSE;
}

int compare_xml_strings(const gpointer left, const gpointer right)
{
    return (xmlStrcmp((const xmlChar*)left, (const xmlChar*)right) == 0);
}

int compare_property_tables(GHashTable *property_table1, GHashTable *property_table2)
{
    return compare_hash_tables(property_table1, property_table2, compare_xml_strings);
}
