#include "servicemapping.h"
#include <nixxml-parse.h>
#include <nixxml-print-nix.h>
#include <nixxml-print-xml.h>
#include "interdependencymapping.h"

gint compare_service_mappings(const ServiceMapping **l, const ServiceMapping **r)
{
    return compare_interdependency_mappings((const InterDependencyMapping **)l, (const InterDependencyMapping **)r);
}

static void *create_service_mapping_from_element(xmlNodePtr element, void *userdata)
{
    return g_malloc0(sizeof(ServiceMapping));
}

static void insert_service_mapping_attributes(void *table, const xmlChar *key, void *value, void *userdata)
{
    ServiceMapping *mapping = (ServiceMapping*)table;

    if(xmlStrcmp(key, (xmlChar*) "service") == 0)
        mapping->service = value;
    else if(xmlStrcmp(key, (xmlChar*) "target") == 0)
        mapping->target = value;
    else if(xmlStrcmp(key, (xmlChar*) "container") == 0)
        mapping->container = value;
    else if(xmlStrcmp(key, (xmlChar*) "containerProvidedByService") == 0)
        mapping->container_provided_by_service = value;
    else
        xmlFree(value);
}

void *parse_service_mapping(xmlNodePtr element, void *userdata)
{
    ServiceMapping *mapping = NixXML_parse_simple_attrset(element, userdata, create_service_mapping_from_element, NixXML_parse_value, insert_service_mapping_attributes);

    /* Set default values */
    if(mapping->target == NULL)
        mapping->target = xmlStrdup((xmlChar*)userdata);

    return mapping;
}

void delete_service_mapping(ServiceMapping *mapping)
{
    if(mapping != NULL)
    {
        xmlFree(mapping->service);
        xmlFree(mapping->container);
        xmlFree(mapping->target);
        xmlFree(mapping->container_provided_by_service);
        g_free(mapping);
    }
}

NixXML_bool check_service_mapping(const ServiceMapping *mapping)
{
    return check_interdependency_mapping((const InterDependencyMapping*)mapping);
}

static void print_service_mapping_attributes_nix(FILE *file, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    ServiceMapping *mapping = (ServiceMapping*)value;

    NixXML_print_attribute_nix(file, "service", mapping->service, indent_level, userdata, NixXML_print_string_nix);
    NixXML_print_attribute_nix(file, "container", mapping->container, indent_level, userdata, NixXML_print_string_nix);
    NixXML_print_attribute_nix(file, "target", mapping->target, indent_level, userdata, NixXML_print_string_nix);
    if(mapping->container_provided_by_service != NULL)
        NixXML_print_attribute_nix(file, "containerProvidedByService", mapping->container_provided_by_service, indent_level, userdata, NixXML_print_string_nix);
}

void print_service_mapping_nix(FILE *file, const ServiceMapping *mapping, const int indent_level, void *userdata)
{
    NixXML_print_attrset_nix(file, mapping, indent_level, userdata, print_service_mapping_attributes_nix, NULL);
}

static void print_service_mapping_attributes_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    ServiceMapping *mapping = (ServiceMapping*)value;

    NixXML_print_simple_attribute_xml(file, "service", mapping->service, indent_level, NULL, userdata, NixXML_print_string_xml);
    NixXML_print_simple_attribute_xml(file, "container", mapping->container, indent_level, NULL, userdata, NixXML_print_string_xml);
    NixXML_print_simple_attribute_xml(file, "target", mapping->target, indent_level, NULL, userdata, NixXML_print_string_xml);
    if(mapping->container_provided_by_service != NULL)
        NixXML_print_simple_attribute_xml(file, "containerProvidedByService", mapping->container_provided_by_service, indent_level, NULL, userdata, NixXML_print_string_xml);
}

void print_service_mapping_xml(FILE *file, const ServiceMapping *mapping, const int indent_level, const char *type_property_name, void *userdata)
{
    NixXML_print_simple_attrset_xml(file, mapping, indent_level, NULL, userdata, print_service_mapping_attributes_xml, NULL);
}
