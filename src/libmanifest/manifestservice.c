#include "manifestservice.h"
#include <nixxml-parse.h>
#include <nixxml-print-nix.h>
#include <nixxml-print-xml.h>
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

void *parse_manifest_service(xmlNodePtr element, void *userdata)
{
    return NixXML_parse_simple_heterogeneous_attrset(element, userdata, create_manifest_service, parse_and_insert_manifest_service_properties);
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

void delete_manifest_service_func(gpointer key, gpointer value, gpointer user_data)
{
    delete_manifest_service((ManifestService*)value);
}

int check_manifest_service(const ManifestService *service)
{
    if(service->name == NULL)
    {
        g_printerr("service.name is not set!\n");
        return FALSE;
    }

    if(service->pkg == NULL)
    {
        g_printerr("service.pkg is not set!\n");
        return FALSE;
    }

    if(service->type == NULL)
    {
        g_printerr("service.type is not set!\n");
        return FALSE;
    }

    return (check_interdependency_mapping_array(service->depends_on) &&
        check_interdependency_mapping_array(service->connects_to));
}

int compare_manifest_services(const gpointer left, const gpointer right)
{
    const ManifestService *service1 = (const ManifestService*)left;
    const ManifestService *service2 = (const ManifestService*)right;

    return (xmlStrcmp(service1->name, service2->name) == 0
      && xmlStrcmp(service1->pkg, service2->pkg) == 0
      && xmlStrcmp(service1->type, service2->type) == 0
      && compare_interdependency_mapping_arrays(service1->depends_on, service2->depends_on)
      && compare_interdependency_mapping_arrays(service1->connects_to, service2->connects_to));
}

static void print_manifest_service_attributes_nix(FILE *file, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    ManifestService *service = (ManifestService*)value;

    NixXML_print_attribute_nix(file, "name", service->name, indent_level, userdata, NixXML_print_string_nix);
    NixXML_print_attribute_nix(file, "pkg", service->pkg, indent_level, userdata, NixXML_print_store_path_nix);
    NixXML_print_attribute_nix(file, "type", service->type, indent_level, userdata, NixXML_print_string_nix);
    NixXML_print_attribute_nix(file, "dependsOn", service->depends_on, indent_level, userdata, print_interdependency_mapping_array_nix);
    NixXML_print_attribute_nix(file, "connectsTo", service->connects_to, indent_level, userdata, print_interdependency_mapping_array_nix);
}

void print_manifest_service_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    NixXML_print_attrset_nix(file, value, indent_level, userdata, print_manifest_service_attributes_nix, NULL);
}

static void print_manifest_service_attributes_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    ManifestService *service = (ManifestService*)value;

    NixXML_print_simple_attribute_xml(file, "name", service->name, indent_level, NULL, userdata, NixXML_print_string_xml);
    NixXML_print_simple_attribute_xml(file, "pkg", service->pkg, indent_level, NULL, userdata, NixXML_print_string_xml);
    NixXML_print_simple_attribute_xml(file, "type", service->type, indent_level, NULL, userdata, NixXML_print_string_xml);
    NixXML_print_simple_attribute_xml(file, "dependsOn", service->depends_on, indent_level, NULL, userdata, print_interdependency_mapping_array_xml);
    NixXML_print_simple_attribute_xml(file, "connectsTo", service->connects_to, indent_level, NULL, userdata, print_interdependency_mapping_array_xml);
}

void print_manifest_service_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_simple_attrset_xml(file, value, indent_level, NULL, userdata, print_manifest_service_attributes_xml, NULL);
}
