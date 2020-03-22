#include "manifestservice.h"
#include <nixxml-parse.h>
#include <nixxml-print-nix.h>
#include <nixxml-print-xml.h>
#include <nixxml-ghashtable.h>
#include <nixxml-glib.h>
#include "interdependencymappingarray.h"
#include "containerstable.h"

static void *create_manifest_service_from_element(xmlNodePtr element, void *userdata)
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
    else if(xmlStrcmp(key, (xmlChar*) "providesContainers") == 0)
        service->provides_containers_table = parse_containers_table(element, userdata);
}

void *parse_manifest_service(xmlNodePtr element, void *userdata)
{
    ManifestService *service = NixXML_parse_simple_heterogeneous_attrset(element, userdata, create_manifest_service_from_element, parse_and_insert_manifest_service_properties);

    /* Set default values */

    if(service != NULL)
    {
        if(service->depends_on == NULL)
            service->depends_on = g_ptr_array_new();
        if(service->connects_to == NULL)
            service->connects_to = g_ptr_array_new();
        if(service->provides_containers_table == NULL)
            service->provides_containers_table = NixXML_create_g_hash_table();
    }

    return service;
}

void delete_manifest_service(ManifestService *service)
{
    if(service != NULL)
    {
        xmlFree(service->name);
        xmlFree(service->pkg);
        xmlFree(service->type);
        delete_interdependency_mapping_array(service->depends_on);
        delete_interdependency_mapping_array(service->connects_to);
        delete_containers_table(service->provides_containers_table);
        g_free(service);
    }
}

int check_manifest_service(const ManifestService *service)
{
    int status = TRUE;

    if(service->name == NULL)
    {
        g_printerr("service.name is not set!\n");
        status = FALSE;
    }

    if(service->pkg == NULL)
    {
        g_printerr("service.pkg is not set!\n");
        status = FALSE;
    }

    if(service->type == NULL)
    {
        g_printerr("service.type is not set!\n");
        status = FALSE;
    }

    if(!check_interdependency_mapping_array(service->depends_on))
        status = FALSE;

    if(!check_interdependency_mapping_array(service->connects_to))
        status = FALSE;

    return status;
}

int compare_manifest_services(const ManifestService *left, const ManifestService *right)
{
    return (xmlStrcmp(left->name, right->name) == 0
      && xmlStrcmp(left->pkg, right->pkg) == 0
      && xmlStrcmp(left->type, right->type) == 0
      && compare_interdependency_mapping_arrays(left->depends_on, right->depends_on)
      && compare_interdependency_mapping_arrays(left->connects_to, right->connects_to)
      && compare_container_tables(left->provides_containers_table, right->provides_containers_table));
}

static void print_manifest_service_attributes_nix(FILE *file, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    ManifestService *service = (ManifestService*)value;

    NixXML_print_attribute_nix(file, "name", service->name, indent_level, userdata, NixXML_print_string_nix);
    NixXML_print_attribute_nix(file, "pkg", service->pkg, indent_level, userdata, NixXML_print_store_path_nix);
    NixXML_print_attribute_nix(file, "type", service->type, indent_level, userdata, NixXML_print_string_nix);
    NixXML_print_attribute_nix(file, "dependsOn", service->depends_on, indent_level, userdata, (NixXML_PrintValueFunc)print_interdependency_mapping_array_nix);
    NixXML_print_attribute_nix(file, "connectsTo", service->connects_to, indent_level, userdata, (NixXML_PrintValueFunc)print_interdependency_mapping_array_nix);
    NixXML_print_attribute_nix(file, "providesContainers", service->provides_containers_table, indent_level, userdata, (NixXML_PrintValueFunc)print_containers_table_nix);
}

void print_manifest_service_nix(FILE *file, const ManifestService *service, const int indent_level, void *userdata)
{
    NixXML_print_attrset_nix(file, service, indent_level, userdata, print_manifest_service_attributes_nix, NULL);
}

static void print_manifest_service_attributes_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    ManifestService *service = (ManifestService*)value;

    NixXML_print_simple_attribute_xml(file, "name", service->name, indent_level, NULL, userdata, NixXML_print_string_xml);
    NixXML_print_simple_attribute_xml(file, "pkg", service->pkg, indent_level, NULL, userdata, NixXML_print_string_xml);
    NixXML_print_simple_attribute_xml(file, "type", service->type, indent_level, NULL, userdata, NixXML_print_string_xml);
    NixXML_print_simple_attribute_xml(file, "dependsOn", service->depends_on, indent_level, NULL, userdata, (NixXML_PrintXMLValueFunc)print_interdependency_mapping_array_xml);
    NixXML_print_simple_attribute_xml(file, "connectsTo", service->connects_to, indent_level, NULL, userdata, (NixXML_PrintXMLValueFunc)print_interdependency_mapping_array_xml);
    NixXML_print_simple_attribute_xml(file, "providesContainers", service->provides_containers_table, indent_level, NULL, userdata, (NixXML_PrintXMLValueFunc)print_containers_table_xml);
}

void print_manifest_service_xml(FILE *file, const ManifestService *service, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_simple_attrset_xml(file, service, indent_level, NULL, userdata, print_manifest_service_attributes_xml, NULL);
}

xmlChar **generate_activation_arguments_for_container_service(const ManifestService *service, const gchar *container_name)
{
    GHashTable *container_properties = g_hash_table_lookup(service->provides_containers_table, container_name);
    return NixXML_generate_env_vars_generic_glib(container_properties);
}
