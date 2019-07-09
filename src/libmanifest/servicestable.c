#include "servicestable.h"
#include <nixxml-parse.h>
#include <nixxml-print-nix.h>
#include <nixxml-ghashtable.h>
#include "interdependencymappingarray.h"

static void *create_manifest_service(xmlNodePtr element, void *userdata)
{
    return g_malloc0(sizeof(ManifestService));
}

static void parse_and_insert_manifest_service_properties(xmlNodePtr element, void *table, const xmlChar *key, void *userdata)
{
    ManifestService *service = (ManifestService*)table;

    if(xmlStrcmp(key, (xmlChar*) "name") == 0)
        service->name = NixXML_parse_value(element, userdata);
    else if(xmlStrcmp(key, (xmlChar*) "pkg") == 0)
        service->pkg = NixXML_parse_value(element, userdata);
    else if(xmlStrcmp(key, (xmlChar*) "type") == 0)
        service->type = NixXML_parse_value(element, userdata);
    else if(xmlStrcmp(key, (xmlChar*) "dependsOn") == 0)
        service->depends_on = parse_interdependency_mapping_array(element, userdata);
    else if(xmlStrcmp(key, (xmlChar*) "connectsTo") == 0)
        service->connects_to = parse_interdependency_mapping_array(element, userdata);
}

static void *parse_manifest_service(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_simple_heterogeneous_attrset(element, userdata, create_manifest_service, parse_and_insert_manifest_service_properties);
}

GHashTable *parse_services_table(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_g_hash_table_verbose(element, "service", "name", userdata, parse_manifest_service);
}

static void delete_manifest_service(ManifestService *service)
{
    xmlFree(service->name);
    xmlFree(service->pkg);
    xmlFree(service->type);
    delete_interdependency_mapping_array(service->depends_on);
    delete_interdependency_mapping_array(service->connects_to);
    g_free(service);
}

void delete_services_table(GHashTable *services_table)
{
    if(services_table != NULL)
    {
        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init(&iter, services_table);
        while(g_hash_table_iter_next(&iter, &key, &value))
            delete_manifest_service((ManifestService*)value);

        g_hash_table_destroy(services_table);
    }
}

static int check_manifest_service(const ManifestService *service)
{
    return(service->name != NULL
        && service->pkg != NULL
        && service->type != NULL
        && check_interdependency_mapping_array(service->depends_on)
        && check_interdependency_mapping_array(service->connects_to));
}

int check_services_table(GHashTable *services_table)
{
    if(services_table == NULL)
        return TRUE;
    else
    {
        GHashTableIter iter;
        gpointer key, value;

        g_hash_table_iter_init(&iter, services_table);
        while(g_hash_table_iter_next(&iter, &key, &value))
        {
            if(!check_manifest_service((ManifestService*)value))
                return FALSE;
        }

        return TRUE;
    }
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

static void print_manifest_service_attributes(FILE *file, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    ManifestService *service = (ManifestService*)value;
    NixXML_print_attribute_nix(file, "name", service->name, indent_level, userdata, NixXML_print_value_nix);
    NixXML_print_attribute_nix(file, "pkg", service->pkg, indent_level, userdata, NixXML_print_value_nix);
    NixXML_print_attribute_nix(file, "type", service->type, indent_level, userdata, NixXML_print_value_nix);
    NixXML_print_attribute_nix(file, "dependsOn", service->depends_on, indent_level, userdata, print_interdependency_mapping_array_nix);
    NixXML_print_attribute_nix(file, "connectsTo", service->connects_to, indent_level, userdata, print_interdependency_mapping_array_nix);
}

static void print_manifest_service_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    NixXML_print_attrset_nix(file, value, indent_level, userdata, print_manifest_service_attributes, NULL);
}

void print_services_table_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    NixXML_print_g_hash_table_nix(file, (GHashTable*)value, indent_level, userdata, print_manifest_service_nix);
}
